/* neuron_tile_display_plugin.cpp
 * This Plugin will tile neuron to display
 * 2014-10-28 : by Hanbo Chen
 */
 
#include "v3d_message.h"
#include <vector>
#include "neuron_tile_display_plugin.h"
#include "basic_surf_objs.h"
#include "../../../v3d_main/neuron_editing/neuron_xforms.h"
using namespace std;
Q_EXPORT_PLUGIN2(neuron_tile_display, neuron_tile_display);
 
QStringList neuron_tile_display::menulist() const
{
	return QStringList() 
		<<tr("neuron_tile_display")
        <<tr("neuron_tile_XY")
        <<tr("neuron_tile_groups")
        <<tr("tile_multi_windows (swc and image files selected)")
        <<tr("tile_multi_windows (swc and image files under one folder)")
        <<tr("set configuration (tile_multi_windows options only)")
        <<tr("about");
}

QStringList neuron_tile_display::funclist() const
{
	return QStringList()
		<<tr("help");
}

QStringList importFileList_addnumbersort(const QString & curFilePath, int method_code)
{
    QStringList myList;
    myList.clear();

    // get the image files namelist in the directory
    QStringList imgSuffix;
    if (method_code ==1)
        imgSuffix<<"*.swc"<<"*.eswc"<<"*.SWC"<<"*.ESWC";
    else if (method_code ==2)
        imgSuffix<<"*.marker";
    else if (method_code ==3)
        imgSuffix<<"*.raw"<<"*.v3draw"<<"*.v3dpbd"<<"*.tif"<<"*.RAW"<<"*.V3DRAW"<<"*.TIF"<<"*.V3DPBD";

    QDir dir(curFilePath);
    if (!dir.exists())
    {
        qWarning("Cannot find the directory");
        return myList;
    }

    foreach (QString file, dir.entryList(imgSuffix, QDir::Files, QDir::Name))
    {
        myList += QFileInfo(dir, file).absoluteFilePath();
    }

    // print filenames
    foreach (QString qs, myList)  qDebug() << qs;

    return myList;
}
void neuron_tile_display::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("neuron_tile_display"))
	{
        dotile(callback, parent);
	}
    else if (menu_name == tr("neuron_tile_XY"))
    {
        doxytile(callback, parent);
    }
    else if (menu_name == tr("neuron_tile_groups"))
    {
        dotile_groups(callback, parent);
    }
    else if (menu_name == tr("tile_multi_windows (swc and image files selected)"))
    {
        QStringList fileList;
        fileList = QFileDialog::getOpenFileNames(0, QObject::tr("Open File"),
                "",
                QObject::tr("Supported file (*.swc *.eswc *.raw *.v3draw *.v3dpbd *.tif)"
                    ));
        if(fileList.isEmpty())
            return;

        unsigned int col=1, row=1, xRez=1, yRez=1;

        QSettings settings("HHMI", "Vaa3D");

        col = settings.value("multi_windows_col").toInt(); if(col<1) col = 1;
        row = settings.value("multi_windows_row").toInt(); if(row<1) row = 1;
        xRez = settings.value("multi_windows_xRez").toInt();if(xRez<1) xRez = 1;
        yRez = settings.value("multi_windows_yRez").toInt();if(yRez<1) yRez = 1;

        V3dR_MainWindow * new3DWindow = NULL;
        int offsety = -1;
        for(V3DLONG i = 0; i < fileList.size(); i++)
        {           
            QString curPathFile = fileList.at(i);
            QFileInfo curfile_info(curPathFile);
            QString cur_suffix = curfile_info.suffix().toUpper();
            if ( (cur_suffix=="LSM") ||
                    (cur_suffix=="TIF") ||
                    (cur_suffix=="TIFF") ||
                    (cur_suffix=="RAW") ||
                    (cur_suffix=="V3DRAW") ||
                    (cur_suffix=="VAA3DRAW") ||
                    (cur_suffix=="RAW5") ||
                    (cur_suffix=="V3DRAW5") ||
                    (cur_suffix=="VAA3DRAW5") ||
                    (cur_suffix=="MRC") ||
                    (cur_suffix=="V3DPBD") ||
                    (cur_suffix=="NRRD") ||
                    (cur_suffix=="NHDR") ||
                    (cur_suffix=="VAA3DPBD") ||
                    (cur_suffix=="MP4") ||
                    (cur_suffix=="H5J"))
            {
                 char * inimg_file  = &curPathFile.toStdString()[0];
                 Image4DSimple* p4DImage = callback.loadImage(inimg_file);
                 v3dhandle newwin = callback.newImageWindow();
                 callback.setImage(newwin, p4DImage);
                 callback.setImageName(newwin,curPathFile);
                 callback.updateImageWindow(newwin);
                 callback.open3DWindow(newwin);
            }
            else if (cur_suffix=="SWC" ||
                     cur_suffix=="ESWC")
            {
                new3DWindow = callback.open3DViewerForSingleSurfaceFile(curPathFile);

            }

            QList <V3dR_MainWindow *> cur_list_3dviewer = callback.getListAll3DViewers();
            for (V3DLONG i = 0; i < cur_list_3dviewer.size(); i++)
            {
                if( (i%col)*xRez ==0)
                    offsety++;
                callback.moveWindow(cur_list_3dviewer.at(i),(i%col+row-1)*xRez,offsety*yRez);
                callback.resizeWindow(cur_list_3dviewer.at(i),xRez,yRez);

            }
        }

    }
    else if (menu_name == tr("tile_multi_windows (swc and image files under one folder)"))
    {
        QString m_InputfolderName = QFileDialog::getExistingDirectory(parent, QObject::tr("Choose the directory including all swc files "),
                                                                      QDir::currentPath(),
                                                                      QFileDialog::ShowDirsOnly);


        unsigned int col=1, row=1, xRez=1, yRez=1;
        QSettings settings("HHMI", "Vaa3D");

        col = settings.value("multi_windows_col").toInt(); if(col<1) col = 1;
        row = settings.value("multi_windows_row").toInt(); if(row<1) row = 1;
        xRez = settings.value("multi_windows_xRez").toInt();if(xRez<1) xRez = 1;
        yRez = settings.value("multi_windows_yRez").toInt();if(yRez<1) yRez = 1;

        QRect deskRect = QApplication::desktop()->availableGeometry();
        qDebug("deskRect height %d and width %d",deskRect.height(),deskRect.width());
        int xRez_o=deskRect.width()/col;
        int yRez_o=deskRect.height()/row;
        yRez_o=yRez_o*3/4;
        qDebug("every col size is %d",xRez_o);
        qDebug("every row size is %d",yRez_o);

        QStringList swcList = importFileList_addnumbersort(m_InputfolderName, 1);
        QStringList imagelist = importFileList_addnumbersort(m_InputfolderName, 3);

        for(V3DLONG i=0; i < imagelist.size(); i++)
        {
            QString curPathIMAGE = imagelist.at(i);
           // v3d_msg(curPathIMAGE);
           // char * inimg_file  = &curPathIMAGE.toStdString()[0];
          //  v3d_msg(QString("%1").arg(inimg_file));
            Image4DSimple* p4DImage = callback.loadImage((char *)curPathIMAGE.toStdString().c_str());
            v3dhandle newwin = callback.newImageWindow();
            callback.setImage(newwin, p4DImage);
            callback.setImageName(newwin,curPathIMAGE);
            callback.updateImageWindow(newwin);
            callback.open3DWindow(newwin);
        }
        V3dR_MainWindow * new3DWindow = NULL;
        int offsety = -1;
        for(V3DLONG i = 0; i < swcList.size(); i++)
        {
            QString curPathSWC = swcList.at(i);
            new3DWindow = callback.open3DViewerForSingleSurfaceFile(curPathSWC);
        }


        QList <V3dR_MainWindow *> cur_list_3dviewer = callback.getListAll3DViewers();
        for (V3DLONG i = 0; i < cur_list_3dviewer.size(); i++)
        {
            if( (i%col)*xRez ==0)
                offsety++;
            callback.moveWindow(cur_list_3dviewer.at(i),(i%col)*(xRez_o+xRez),offsety*(yRez_o+yRez));
            callback.resizeWindow(cur_list_3dviewer.at(i),xRez_o,yRez_o);
        }


    }
    else if (menu_name == tr("set configuration (tile_multi_windows options only)"))
    {
             bool ok1, ok2, ok3, ok4;
             unsigned int col=1, row=1, xRez=1, yRez=1;

             col = QInputDialog::getInteger(parent, "",
                                           "#tiles per row:",
                                           1, 1, 1000, 1, &ok1);

             if(ok1)
             {
                 row = QInputDialog::getInteger(parent, "",
                                               "display from column:",
                                               1, 1, col, 1, &ok2);
             }
             else
                 return;

             if(ok2)
             {
                 xRez = QInputDialog::getInteger(parent, " ",
                                               "offset X:",
                                               1, -10000, 10000, 1, &ok3);
             }
             else
                 return;

             if(ok3)
             {
                 yRez = QInputDialog::getInteger(parent, " ",
                                               "offset Y:",
                                               1, -10000, 10000, 1, &ok4);
             }
             else
                 return;

             QSettings settings("HHMI", "Vaa3D");
             settings.setValue("multi_windows_col", col);
             settings.setValue("multi_windows_row", row);
             settings.setValue("multi_windows_xRez", xRez);
             settings.setValue("multi_windows_yRez", yRez);

             v3d_msg("Configuration Done!");
    }
    else
	{
		v3d_msg(tr("This Plugin will tile neuron to display. "
			"Developed by Hanbo Chen, 2014-10-28"));
	}
}

bool neuron_tile_display::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

void neuron_tile_display::dotile(V3DPluginCallback2 &callback, QWidget *parent)
{
    //select the window to operate
    QList <V3dR_MainWindow *> allWindowList = callback.getListAll3DViewers();
    QList <V3dR_MainWindow *> selectWindowList;
    V3dR_MainWindow * v3dwin;
    QList<NeuronTree> * ntTreeList;
    int winid;
    qDebug("search for 3D windows");
    for (V3DLONG i=0;i<allWindowList.size();i++)
    {
        ntTreeList = callback.getHandleNeuronTrees_Any3DViewer(allWindowList[i]);
        if(ntTreeList->size()>1)
            selectWindowList.append(allWindowList[i]);
    }
    qDebug("match and select 3D windows");
    if(selectWindowList.size()<1){
        v3d_msg("Cannot find 3D view with SWC file. Please load the SWC files you want to tile in the same 3D view");
        return;
    }else if(selectWindowList.size()>1){
        //pop up a window to select

        QStringList items;
        for(int i=0; i<selectWindowList.size(); i++){
            items.append(callback.getImageName(selectWindowList[i]));
        }
        bool ok;
        QString selectitem = QInputDialog::getItem(parent, QString::fromUtf8("Neuron Tile Viewer"), QString::fromUtf8("Select A Window to Operate"), items, 0, false, &ok);
        if(!ok) return;
        for(int i=0; i<selectWindowList.size(); i++){
            if(selectitem==callback.getImageName(selectWindowList[i]))
            {
                winid=i;
                break;
            }
        }
    }else{
        winid=0;
    }
    v3dwin = selectWindowList[winid];

    NeuronTileDisplayDialog * myDialog = NULL;
    myDialog = new NeuronTileDisplayDialog(&callback, v3dwin);
    myDialog->show();
}


void neuron_tile_display::dotile_groups(V3DPluginCallback2 &callback, QWidget *parent)
{
    QStringList ano_file_list = QFileDialog::getOpenFileNames(parent,"ano files",
                      QDir::currentPath(),"ano files (*.ano);;All files (*.*)" );


    V3dR_MainWindow * surface_win = callback.createEmpty3DViewer();
    if (!surface_win)
    {
        v3d_msg(QString("Failed to open an empty window!"));
        return;
    }
    callback.setWindowDataTitle(surface_win, QFileInfo(ano_file_list[0]).absoluteDir().absolutePath());
    NeuronTileGroupsDialog * myDialog = NULL;
    myDialog = new NeuronTileGroupsDialog(&callback, surface_win);
    myDialog->setAnoFileList(ano_file_list);
    myDialog->show();

}


void neuron_tile_display::doxytile(V3DPluginCallback2 &callback, QWidget *parent)
{
    //select the window to operate
    QList <V3dR_MainWindow *> allWindowList = callback.getListAll3DViewers();
    QList <V3dR_MainWindow *> selectWindowList;
    V3dR_MainWindow * v3dwin;
    QList<NeuronTree> * ntTreeList;
    int winid;
    qDebug("search for 3D windows");
    for (V3DLONG i=0;i<allWindowList.size();i++)
    {
        ntTreeList = callback.getHandleNeuronTrees_Any3DViewer(allWindowList[i]);
        if(ntTreeList->size()>1)
            selectWindowList.append(allWindowList[i]);
    }
    qDebug("match and select 3D windows");
    if(selectWindowList.size()<1){
        v3d_msg("Cannot find 3D view with SWC file. Please load the SWC files you want to tile in the same 3D view");
        return;
    }else if(selectWindowList.size()>1){
        //pop up a window to select

        QStringList items;
        for(int i=0; i<selectWindowList.size(); i++){
            items.append(callback.getImageName(selectWindowList[i]));
        }
        bool ok;
        QString selectitem = QInputDialog::getItem(parent, QString::fromUtf8("Neuron Tile Viewer"), QString::fromUtf8("Select A Window to Operate"), items, 0, false, &ok);
        if(!ok) return;
        for(int i=0; i<selectWindowList.size(); i++){
            if(selectitem==callback.getImageName(selectWindowList[i]))
            {
                winid=i;
                break;
            }
        }
    }else{
        winid=0;
    }
    v3dwin = selectWindowList[winid];

    NeuronXYTileDialog * myDialog = NULL;
    myDialog = new NeuronXYTileDialog(&callback, v3dwin);
    myDialog->show();
}
