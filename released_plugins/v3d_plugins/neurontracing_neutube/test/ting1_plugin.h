/* ting1_plugin.h
 * a collaboration with Hanchuan
 * 2012-12-9 : by Ting Zhao and HP
 */
 
#ifndef __TING1_PLUGIN_H__
#define __TING1_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include "z3dapplication.h"


class MainWindow;

class neutube : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
    neutube();
    ~neutube();
    float getPluginVersion() const {return 0.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
private:
    MainWindow *m_mainWindow;
    Z3DApplication m_3dApp;
};


#endif

