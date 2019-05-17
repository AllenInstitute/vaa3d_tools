/*
Test_plugin.h
Sat Apr 27 17:25:42 2019
*/

#ifndef __TEST_PLUGIN_H__
#define __TEST_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class TestPlugin: public QObject, public V3DPluginInterface2_1
{
   Q_OBJECT
   Q_INTERFACES(V3DPluginInterface2_1);

public:
   float getPluginVersion() const{return 1.1f;}

   QStringList menulist() const;
   void domenu(const QString & menu_name, V3DPluginCallback2 & callback, QWidget * parent);

   QStringList funclist() const;
   bool dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback, QWidget * parent);
};

#endif
