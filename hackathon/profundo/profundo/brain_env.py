#!/usr/bin/env python
# -*- coding: utf-8 -*-
# File: brain_env.py
# Author: Amir Alansary <amiralansary@gmail.com>

import csv
import itertools


def warn(*args, **kwargs):
    pass


import warnings

warnings.warn = warn
warnings.simplefilter("ignore", category=PendingDeprecationWarning)

import os
import sys
import six
import random
import threading
import numpy as np
from tensorpack import logger
from collections import (Counter, defaultdict, deque, namedtuple)

import cv2
import math
import time
from PIL import Image
import subprocess
import shutil

import gym
from gym import spaces


from tensorpack.utils.utils import get_rng
from tensorpack.utils.stats import StatCounter

from IPython.core.debugger import set_trace
from sampleTrain import FilesListCubeNPY
from sklearn.metrics import jaccard_similarity_score

__all__ = ['Brain_Env', 'FrameStack']

_ALE_LOCK = threading.Lock()

ObservationBounds = namedtuple('ObservationBounds', ['xmin', 'xmax', 'ymin', 'ymax', 'zmin', 'zmax'])


# ===================================================================
# =================== 3d medical environment ========================
# ===================================================================

class Brain_Env(gym.Env):
    """Class that provides 3D medical image environment.
    This is just an implementation of the classic "agent-environment loop".
    Each time-step, the agent chooses an action, and the environment returns
    an observation and a reward."""

    def __init__(self, directory=None, viz=False, train=False, files_list=None,
                 screen_dims=(27, 27, 27), multiscale=False,
                 max_num_frames=0, saveGif=False, saveVideo=False):
        """
        :param train_directory: environment or game name
        :param viz: visualization
            set to 0 to disable
            set to +ve number to be the delay between frames to show
            set to a string to be the directory for storing frames
        :param screen_dims: shape of the frame cropped from the image to feed
            it to dqn (d,w,h) - defaults (27,27,27)
        :param nullop_start: start with random number of null ops
        :param location_history_length: consider lost of lives as end of
            episode (useful for training)
        :max_num_frames: maximum number of frames per episode.
        """
        # python DQN_midl2018.py --task play --gpu 0 --algo DuelingDouble --load train_log/DuelDoubleDQN_multiscale_brain_mri_point_ac_ROI_45_45_45_midl2018/model-350000

        # ######################################################################
        # ## generate evaluation results from 19 different points
        # ## save results in csv file
        # self.csvfile = 'DuelDoubleDQN_multiscale_brain_mri_point_pc_ROI_45_45_45_midl2018.csv'
        # if not train:
        #     with open(self.csvfile, 'w') as outcsv:
        #         fields = ["filename", "dist_error"]
        #         writer = csv.writer(outcsv)
        #         writer.writerow(map(lambda x: x, fields))
        #
        # x = [0.5,0.25,0.75]
        # y = [0.5,0.25,0.75]
        # z = [0.5,0.25,0.75]
        # self.start_points = []
        # for combination in itertools.product(x, y, z):
        #     if 0.5 in combination: self.start_points.append(combination)
        # self.start_points = itertools.cycle(self.start_points)
        # self.count_points = 0
        # self.total_loc = []
        # ######################################################################

        super(Brain_Env, self).__init__()

        # inits stat counters
        self.reset_stat()

        # counter to limit number of steps per episodes
        self.cnt = 0
        # maximum number of frames (steps) per episodes
        self.max_num_frames = max_num_frames
        # stores information: terminal, score, distError
        self.info = None
        # option to save display as gif
        self.saveGif = saveGif
        self.saveVideo = saveVideo
        # training flag
        self.train = train
        # image dimension (2D/3D)
        self.screen_dims = screen_dims
        self.dims = len(self.screen_dims)
        # multi-scale agent
        self.multiscale = multiscale
        # FIXME force multiscale false for now
        self.multiscale = False

        # init env dimensions
        if self.dims == 2:
            self.width, self.height = screen_dims
        else:
            self.width, self.height, self.depth = screen_dims

        with _ALE_LOCK:
            self.rng = get_rng(self)
            # visualization setup
            if isinstance(viz, six.string_types):  # check if viz is a string
                assert os.path.isdir(viz), viz
                viz = 0
            if isinstance(viz, int):
                viz = float(viz)
            self.viz = viz
            if self.viz and isinstance(self.viz, float):
                self.viewer = None
                self.gif_buffer = []
        # stat counter to store current score or accumlated reward
        self.current_episode_score = StatCounter()
        # get action space and minimal action set
        self.action_space = spaces.Discrete(6)  # change number actions here
        self.actions = self.action_space.n
        self.observation_space = spaces.Box(low=0, high=255,
                                            shape=self.screen_dims)
        # history buffer for storing last locations to check oscilations
        self._history_length = max_num_frames
        # TODO initialize _observation_bounds limits from input image coordinates
        self._observation_bounds = ObservationBounds(0, 0, 0, 0, 0, 0)
        # add your data loader here
        # self.files = filesListBrainMRLandmark(directory,files_list)
        self.files = FilesListCubeNPY(directory, files_list)
        # self.files = filesListCardioMRLandmark(directory,files_list)
        # prepare file sampler
        self.filepath = None
        self.file_sampler = self.files.sample_circular()  # returns generator
        # reset buffer, terminal, counters, and init new_random_game
        self._restart_episode()

    def reset(self):
        # with _ALE_LOCK:
        self._restart_episode()
        return self._observe()

    def _restart_episode(self):
        """
        restart current episode
        """
        self.terminal = False
        self.cnt = 0  # counter to limit number of steps per episodes
        self.num_games.feed(1)
        self.current_episode_score.reset()  # reset the stat counter
        self._clear_history()
        self.new_random_game()

    def new_random_game(self):
        """
        load image,
        set dimensions,
        randomize start point,
        init _screen, qvals,
        calc distance to goal
        """
        self.terminal = False
        self.viewer = None
        # ######################################################################
        # ## generate evaluation results from 19 different points
        # if self.count_points ==0:
        #     print('\n============== new game ===============\n')
        #     # save results
        #     if self.total_loc:
        #         with open(self.csvfile, 'a') as outcsv:
        #             fields= [self.filename, self.cur_dist]
        #             writer = csv.writer(outcsv)
        #             writer.writerow(map(lambda x: x, fields))
        #         self.total_loc = []
        #     # sample a new image
        #     self._state, self._target_loc, self.filepath, self.spacing = next(self.sampled_files)
        #     scale = next(self.start_points)
        #     self.count_points +=1
        # else:
        #     self.count_points += 1
        #     logger.info('count_points {}'.format(self.count_points))
        #     scale = next(self.start_points)
        #
        # x = int(scale[0] * self._state.dims[0])
        # y = int(scale[1] * self._state.dims[1])
        # z = int(scale[2] * self._state.dims[2])
        # logger.info('starting point {}-{}-{}'.format(x,y,z))
        # ######################################################################

        # # sample a new image
        self.filepath, self.filename= next(self.file_sampler)
        self._state = np.load(self.filepath)
        self.original_state = np.copy(self._state)

        # multiscale (e.g. start with 3 -> 2 -> 1)
        # scale can be thought of as sampling stride
        if self.multiscale:
            ## brain
            self.action_step = 9
            self.xscale = 3
            self.yscale = 3
            self.zscale = 3
            ## cardiac
            # self.action_step = 6
            # self.xscale = 2
            # self.yscale = 2
            # self.zscale = 2
        else:
            self.action_step = 1
            self.xscale = 1
            self.yscale = 1
            self.zscale = 1
        # image volume size
        self._image_dims = np.shape(self._state)
        #######################################################################
        ## select random starting point
        # add padding to avoid start right on the border of the image
        if self.train:
            skip_thickness = (int(self._image_dims[0] / 5),
                              int(self._image_dims[1] / 5),
                              int(self._image_dims[2] / 5))
        else:
            skip_thickness = (int(self._image_dims[0] / 4),
                              int(self._image_dims[1] / 4),
                              int(self._image_dims[2] / 4))

        x = self.rng.randint(0 + skip_thickness[0],
                             self._image_dims[0] - skip_thickness[0])
        y = self.rng.randint(0 + skip_thickness[1],
                             self._image_dims[1] - skip_thickness[1])
        z = self.rng.randint(0 + skip_thickness[2],
                             self._image_dims[2] - skip_thickness[2])
        #######################################################################

        self._location = (x, y, z)
        self._start_location = (x, y, z)
        self._qvalues = [0, ] * self.actions
        self._observation = self._observe()
        self.cur_IOU = 0.0

    def calc_IOU(self):
        """ calculate the Intersection over Union AKA Jaccard Index
        between two images

        https://en.wikipedia.org/wiki/Jaccard_index
        """
        # flatten bc  jaccard_similarity_score expects 1D arrays
        state = self._state.ravel()
        state[state != -1] = 0  # mask out non-agent trajectory
        state = state.astype(bool)  # everything non-zero = True
        original_state = self.original_state.astype(bool)
        iou = jaccard_similarity_score(state, original_state.ravel())
        return iou

    def step(self, act, qvalues):
        """The environment's step function returns exactly what we need.
        Args:
          act:
        Returns:
          observation (object):
            an environment-specific object representing your observation of
            the environment. For example, pixel data from a camera, joint angles
            and joint velocities of a robot, or the board state in a board game.
          reward (float):
            amount of reward achieved by the previous action. The scale varies
            between environments, but the goal is always to increase your total
            reward.
          done (boolean):
            whether it's time to reset the environment again. Most (but not all)
            tasks are divided up into well-defined episodes, and done being True
            indicates the episode has terminated. (For example, perhaps the pole
            tipped too far, or you lost your last life.)
          info (dict):
            diagnostic information useful for debugging. It can sometimes be
            useful for learning (for example, it might contain the raw
            probabilities behind the environment's last state change). However,
            official evaluations of your agent are not allowed to use this for
            learning.
        """
        self._qvalues = qvalues
        current_loc = self._location
        self.terminal = False
        go_out = False

        # UP Z+ -----------------------------------------------------------
        if (act == 0):
            next_location = (current_loc[0],
                             current_loc[1],
                             round(current_loc[2] + self.action_step))
            if (next_location[2] >= self._image_dims[2]):
                # print(' trying to go out the image Z+ ',)
                next_location = current_loc
                go_out = True

        # FORWARD Y+ ---------------------------------------------------------
        if (act == 1):
            next_location = (current_loc[0],
                             round(current_loc[1] + self.action_step),
                             current_loc[2])
            if (next_location[1] >= self._image_dims[1]):
                # print(' trying to go out the image Y+ ',)
                next_location = current_loc
                go_out = True
        # RIGHT X+ -----------------------------------------------------------
        if (act == 2):
            next_location = (round(current_loc[0] + self.action_step),
                             current_loc[1],
                             current_loc[2])
            if next_location[0] >= self._image_dims[0]:
                # print(' trying to go out the image X+ ',)
                next_location = current_loc
                go_out = True
        # LEFT X- -----------------------------------------------------------
        if act == 3:
            next_location = (round(current_loc[0] - self.action_step),
                             current_loc[1],
                             current_loc[2])
            if next_location[0] <= 0:
                # print(' trying to go out the image X- ',)
                next_location = current_loc
                go_out = True
        # BACKWARD Y- ---------------------------------------------------------
        if act == 4:
            next_location = (current_loc[0],
                             round(current_loc[1] - self.action_step),
                             current_loc[2])
            if next_location[1] <= 0:
                # print(' trying to go out the image Y- ',)
                next_location = current_loc
                go_out = True
        # DOWN Z- -----------------------------------------------------------
        if act == 5:
            next_location = (current_loc[0],
                             current_loc[1],
                             round(current_loc[2] - self.action_step))
            if next_location[2] <= 0:
                # print(' trying to go out the image Z- ',)
                next_location = current_loc
                go_out = True
        # ---------------------------------------------------------------------
        # ---------------------------------------------------------------------
        # punish -1 reward if the agent tries to go out
        if go_out:
            self.reward = -1
        else:
            self.reward = self._calc_reward()
        # update screen, reward ,location, terminal
        self._location = next_location
        self._observation = self._observe()

        # terminate if the distance is less than 1 during trainig
        if self.train:
            if self.cur_IOU <= 1:
                self.terminal = True
                self.num_success.feed(1)

        # terminate if maximum number of steps is reached
        self.cnt += 1
        if self.cnt >= self.max_num_frames: self.terminal = True

        # update history buffer with new location and qvalues
        self.cur_IOU = self.calc_IOU()
        self._update_history()

        # check if agent oscillates
        if self._oscillate:
            # TODO: rewind history, recalculate IOU
            self._location = self.getBestLocation()  # TODO replace
            self._observation = self._observe()
            self.cur_IOU = self.calc_IOU()
            # multi-scale steps
            # if self.multiscale:
            #     if self.xscale > 1:
            #         self.xscale -= 1
            #         self.yscale -= 1
            #         self.zscale -= 1
            #         self.action_step = int(self.action_step / 3)
            #         self._clear_history()
            #     # terminate if scale is less than 1
            #     else:
            #         self.terminal = True
            #         if self.cur_IOU >= 0.9: self.num_success.feed(1)
            # else:
            self.terminal = True
            if self.cur_IOU >= 0.9: self.num_success.feed(1)

        # render screen if viz is on
        with _ALE_LOCK:
            if self.viz:
                if isinstance(self.viz, float):
                    self.display()

        distance_error = self.cur_IOU
        self.current_episode_score.feed(self.reward)

        info = {'score': self.current_episode_score.sum, 'gameOver': self.terminal, 'distError': distance_error,
                'filename': self.filename}

        # #######################################################################
        # ## generate evaluation results from 19 different points
        # if self.terminal:
        #     logger.info(info)
        #     self.total_loc.append(self._location)
        #     if not(self.count_points == 19):
        #         self._restart_episode()
        #     else:
        #         mean_location = np.mean(self.total_loc,axis=0)
        #         logger.info('total_loc {} \n mean_location{}'.format(self.total_loc, mean_location))
        #         self.cur_dist = self.calcDistance(mean_location,
        #                                           self._target_loc,
        #                                           self.spacing)
        #         logger.info('final distance error {} \n'.format(self.cur_dist))
        #         self.count_points = 0
        # #######################################################################

        return self._observe(), self.reward, self.terminal, info


    def _clear_history(self):
        """ clear history buffer with current state
        """
        self._agent_nodes = [(0,) * self.dims] * self._history_length
        self._IOU_history = np.zeros(self._history_length)
        # list of q-value lists
        self._qvalues_history = [(0,) * self.actions] * self._history_length

    def _update_history(self):
        """ update history buffer with current state
        """
        # update location history
        self._agent_nodes[self.cnt] = self._location
        # update jaccard index history
        self._IOU_history[self.cnt] = self.curr_IOU
        # update q-value history
        self._qvalues_history[self.cnt] = self._qvalues

    def _observe(self):
        """
        crop image data around current location to update what network sees.
        update _observation_bounds

        :return: new state
        """
        # initialize screen with zeros - all background
        observation = np.zeros((self.screen_dims))

        # screen uses coordinate system relative to origin (0, 0, 0)
        screen_xmin, screen_ymin, screen_zmin = 0, 0, 0
        screen_xmax, screen_ymax, screen_zmax = self.screen_dims

        # extract boundary locations using coordinate system relative to "global" image
        # width, height, depth in terms of screen coord system
        if self.xscale % 2:
            xmin = self._location[0] - int(self.width * self.xscale / 2) - 1
            xmax = self._location[0] + int(self.width * self.xscale / 2)
            ymin = self._location[1] - int(self.height * self.yscale / 2) - 1
            ymax = self._location[1] + int(self.height * self.yscale / 2)
            zmin = self._location[2] - int(self.depth * self.zscale / 2) - 1
            zmax = self._location[2] + int(self.depth * self.zscale / 2)
        else:
            xmin = self._location[0] - round(self.width * self.xscale / 2)
            xmax = self._location[0] + round(self.width * self.xscale / 2)
            ymin = self._location[1] - round(self.height * self.yscale / 2)
            ymax = self._location[1] + round(self.height * self.yscale / 2)
            zmin = self._location[2] - round(self.depth * self.zscale / 2)
            zmax = self._location[2] + round(self.depth * self.zscale / 2)

        # check if they violate image boundary and fix it
        if xmin < 0:
            xmin = 0
            screen_xmin = screen_xmax - len(np.arange(xmin, xmax, self.xscale))
        if ymin < 0:
            ymin = 0
            screen_ymin = screen_ymax - len(np.arange(ymin, ymax, self.yscale))
        if zmin < 0:
            zmin = 0
            screen_zmin = screen_zmax - len(np.arange(zmin, zmax, self.zscale))
        if xmax > self._image_dims[0]:
            xmax = self._image_dims[0]
            screen_xmax = len(np.arange(xmin, xmax, self.xscale))
        if ymax > self._image_dims[1]:
            ymax = self._image_dims[1]
            screen_ymax = len(np.arange(ymin, ymax, self.yscale))
        if zmax > self._image_dims[2]:
            zmax = self._image_dims[2]
            screen_zmax = len(np.arange(zmin, zmax, self.zscale))

        # TODO: take image, mask it w agent trajectory
        # agent_trajectory = self.trajectory_to_branch()
        # self._state = np.copyto(self.original_state, agent_trajectory, where=True)


        # crop image data to update what network sees
        # image coordinate system becomes screen coordinates
        # scale can be thought of as a stride
        print(xmin, xmax, ymin, ymax, zmin, zmax)
        observation[screen_xmin:screen_xmax, screen_ymin:screen_ymax, screen_zmin:screen_zmax] = self._state[
                                                                                            xmin:xmax,
                                                                                            ymin:ymax,
                                                                                            zmin:zmax]

        # update _observation_bounds limits from input image coordinates
        # this is what the network sees
        self._observation_bounds = ObservationBounds(xmin, xmax,
                                                     ymin, ymax,
                                                     zmin, zmax)

        return observation

    def trajectory_to_branch(self):
        """take location histroy, generate connected branches"""


    def _calc_reward(self):
        """ Calculate the new reward based on the increase in IoU
        TODO: move IOU update to updator
        TODO use IOU history
        TODO: if current location is same as past location, always penalize (discourage retracing)
        """
        next_IOU = self.calc_IOU()
        IOU_difference = self.cur_IOU - next_IOU
        self.cur_IOU = next_IOU
        return IOU_difference

    @property
    def _oscillate(self):
        """ Return True if the agent is stuck and oscillating
        """
        # TODO reimplement
        # TODO: erase last few frames if oscillation is detected
        counter = Counter(self._agent_nodes)
        freq = counter.most_common()

        # TODO: wtF?
        if freq[0][0] == (0, 0, 0):
            if (freq[1][1] > 3):
                return True
            else:
                return False
        elif (freq[0][1] > 3):
            return True

    def get_action_meanings(self):
        """ return array of integers for actions"""
        ACTION_MEANING = {
            1: "UP",  # MOVE Z+
            2: "FORWARD",  # MOVE Y+
            3: "RIGHT",  # MOVE X+
            4: "LEFT",  # MOVE X-
            5: "BACKWARD",  # MOVE Y-
            6: "DOWN",  # MOVE Z-
        }
        return [ACTION_MEANING[i] for i in self.actions]

    @property
    def getScreenDims(self):
        """
        return screen dimensions
        """
        return (self.width, self.height, self.depth)

    def _locations_to_branch(self):
        # TODO implement
        locs = self._location

    def lives(self):
        return None

    def reset_stat(self):
        """ Reset all statistics counter"""
        self.stats = defaultdict(list)
        self.num_games = StatCounter()
        self.num_success = StatCounter()

    # def display(self, return_rgb_array=False):
    #     # pass
    #     # get dimensions
    #     current_point = self._location
    #     target_point = self._target_loc
    #     # get image and convert it to pyglet
    #     plane = self.get_plane(current_point[2])  # z-plane
    #     # plane = np.squeeze(self._current_state()[:,:,13])
    #     img = cv2.cvtColor(plane, cv2.COLOR_GRAY2RGB)  # congvert to rgb
    #     # rescale image
    #     # INTER_NEAREST, INTER_LINEAR, INTER_AREA, INTER_CUBIC, INTER_LANCZOS4
    #     scale_x = 1
    #     scale_y = 1
    #     #
    #     # img = cv2.resize(img,
    #     #                  (int(scale_x*img.shape[1]),int(scale_y*img.shape[0])),
    #     #                  interpolation=cv2.INTER_LINEAR)
    #     # skip if there is a viewer open
    #
    #     # TODO: replace viewer code
    #     if (not self.viewer) and self.viz:
    #         from viewer import SimpleImageViewer
    #         self.viewer = SimpleImageViewer(arr=img,
    #                                         scale_x=1,
    #                                         scale_y=1,
    #                                         filepath=self.filename)
    #         self.gif_buffer = []
    #     # display image
    #     self.viewer.draw_image(img)
    #     # draw a transparent circle around target point with variable radius
    #     # based on the difference z-direction
    #     diff_z = scale_x * abs(current_point[2] - target_point[2])
    #     self.viewer.draw_circle(radius=diff_z,
    #                             pos_x=scale_x * target_point[0],
    #                             pos_y=scale_y * target_point[1],
    #                             color=(1.0, 0.0, 0.0, 0.2))
    #     # draw target point
    #     self.viewer.draw_circle(radius=scale_x * 1,
    #                             pos_x=scale_x * target_point[0],
    #                             pos_y=scale_y * target_point[1],
    #                             color=(1.0, 0.0, 0.0, 1.0))
    #     # draw current point
    #     self.viewer.draw_circle(radius=scale_x * 1,
    #                             pos_x=scale_x * current_point[0],
    #                             pos_y=scale_y * current_point[1],
    #                             color=(0.0, 0.0, 1.0, 1.0))
    #     # draw a box around the agent - what the network sees ROI
    #     self.viewer.draw_rect(self._observation_bounds.xmin, self._observation_bounds.ymin,
    #                           self._observation_bounds.xmax, self._observation_bounds.ymax)
    #     self.viewer.display_text('Agent ', color=(204, 204, 0, 255),
    #                              x=self._observation_bounds.xmin - 15,
    #                              y=self._observation_bounds.ymin)
    #     # display info
    #     color = (0, 204, 0, 255) if self.reward > 0 else (204, 0, 0, 255)
    #     text = 'Error ' + str(round(self.cur_dist, 3)) + 'mm'
    #     self.viewer.display_text(text, color=color, x=10, y=20)
    #     text = 'Spacing ' + str(self.xscale)
    #     self.viewer.display_text(text, color=color,
    #                              x=10, y=self._image_dims[1] - 80)
    #
    #     # render and wait (viz) time between frames
    #     self.viewer.render()
    #     # time.sleep(self.viz)
    #     # save gif
    #     if self.saveGif:
    #         image_data = pyglet.image.get_buffer_manager().get_color_buffer().get_image_data()
    #         data = image_data.get_data('RGB', image_data.width * 3)
    #         arr = np.array(bytearray(data)).astype('uint8')
    #         arr = np.flip(np.reshape(arr, (image_data.height, image_data.width, -1)), 0)
    #         im = Image.fromarray(arr)
    #         self.gif_buffer.append(im)
    #
    #         if not self.terminal:
    #             gifname = self.filename.split('.')[0] + '.gif'
    #             self.viewer.saveGif(gifname, arr=self.gif_buffer,
    #                                 duration=self.viz)
    #     if self.saveVideo:
    #         dirname = 'tmp_video'
    #         if self.cnt <= 1:
    #             if os.path.isdir(dirname):
    #                 logger.warn("""Log directory {} exists! Use 'd' to delete it. """.format(dirname))
    #                 act = input("select action: d (delete) / q (quit): ").lower().strip()
    #                 if act == 'd':
    #                     shutil.rmtree(dirname, ignore_errors=True)
    #                 else:
    #                     raise OSError("Directory {} exits!".format(dirname))
    #             os.mkdir(dirname)
    #
    #         frame = dirname + '/' + '%04d' % self.cnt + '.png'
    #         pyglet.image.get_buffer_manager().get_color_buffer().save(frame)
    #         if self.terminal:
    #             resolution = str(3 * self.viewer.img_width) + 'x' + str(3 * self.viewer.img_height)
    #             save_cmd = ['ffmpeg', '-f', 'image2', '-framerate', '30',
    #                         '-pattern_type', 'sequence', '-start_number', '0', '-r',
    #                         '6', '-i', dirname + '/%04d.png', '-s', resolution,
    #                         '-vcodec', 'libx264', '-b:v', '2567k', self.filename + '.mp4']
    #             subprocess.check_output(save_cmd)
    #             shutil.rmtree(dirname, ignore_errors=True)


# class DiscreteActionSpace(object):
#
#     def __init__(self, num):
#         super(DiscreteActionSpace, self).__init__()
#         self.num = num
#         self.rng = get_rng(self)
#
#     def sample(self):
#         return self.rng.randint(self.num)
#
#     def num_actions(self):
#         return self.num
#
#     def __repr__(self):
#         return "DiscreteActionSpace({})".format(self.num)
#
#     def __str__(self):
#         return "DiscreteActionSpace({})".format(self.num)


# =============================================================================
# ================================ FrameStack =================================
# =============================================================================
class FrameStack(gym.Wrapper):
    """used when not training. wrapper for Medical Env"""
    def __init__(self, env, k):
        """Buffer observations and stack across channels (last axis)."""
        gym.Wrapper.__init__(self, env)
        self.k = k  # history length
        self.frames = deque([], maxlen=k)
        shp = env.observation_space.shape
        self._base_dim = len(shp)
        new_shape = shp + (k,)
        self.observation_space = spaces.Box(low=0, high=255, shape=new_shape)

    def _reset(self):
        """Clear buffer and re-fill by duplicating the first observation."""
        ob = self.env.reset()
        for _ in range(self.k - 1):
            self.frames.append(np.zeros_like(ob))
        self.frames.append(ob)
        return self._observation()

    def step(self, action, q_values):
        ob, reward, done, info = self.env.step(action, q_values)
        self.frames.append(ob)
        return self._observation(), reward, done, info

    def _observation(self):
        assert len(self.frames) == self.k
        return np.stack(self.frames, axis=-1)
        # if self._base_dim == 2:
        #     return np.stack(self.frames, axis=-1)
        # else:
        #     return np.concatenate(self.frames, axis=2)


# =============================================================================
# ================================== notes ====================================
# =============================================================================
"""

## Notes from landmark detection Siemens paper
# states  -> ROI - center current pos - size (2D 60x60) (3D 26x26x26)
# actions -> move (up, down, left, right)
# rewards -> delta(d) relative distance change after executing a move (action)

# re-sample -> isotropic (2D 2mm) (3D 1mm)

# gamma = 0.9 , replay memory size P = 100000 , learning rate = 0.00025
# net : 3 conv+pool - 3 FC+dropout (3D kernels for 3d data)

# navigate till oscillation happen (terminate when infinite loop)

# location is a high-confidence landmark -> if the expected reward from this location is max(q*(s_target,a))<1 the agent is closer than one pixel

# object is not in the image: oscillation occurs at points where max(q)>4


## Other Notes:

    DeepMind's original DQN paper
        used frame skipping (for fast playing/learning) and
        applied pixel-wise max to consecutive frames (to handle flickering).

    so an input to the neural network is consisted of four frame;
        [max(T-1, T), max(T+3, T+4), max(T+7, T+8), max(T+11, T+12)]

    ALE provides mechanism for frame skipping (combined with adjustable random action repeat) and color averaging over skipped frames. This is also used in simple_dqn's ALEEnvironment

    Gym's Atari Environment has built-in stochastic frame skipping common to all games. So the frames returned from environment are not consecutive.

    The reason behind Gym's stochastic frame skipping is, as mentioned above, to make environment stochastic. (I guess without this, the game will be completely deterministic?)
    cf. in original DQN and simple_dqn same randomness is achieved by having agent performs random number of dummy actions at the beginning of each episode.

    I think if you want to reproduce the behavior of the original DQN paper, the easiest will be disabling frame skip and color averaging in ALEEnvironment then construct the mechanism on agent side.


"""
