#pragma once

#include <QTimer>
#include <QWidget>

#include "ECCutDATA/Block3DForCut.h"
#include "ECCutDATA/PointForCut.h"
#include "DisplayBlock3DForCutsInOneNodeJoint.h"

class vtkRenderer;
class QVTKOpenGLWidget;

class DisplayCuttingResultWindow : public QWidget
{
    Q_OBJECT

public:
    explicit DisplayCuttingResultWindow(QWidget *parent = nullptr);
    ~DisplayCuttingResultWindow();

public slots:
    void render();

protected:
    void _initVTKWidget();

    void _readData();
    void _processTexture();
    std::string _readGlslFile(const char* path);
private:
    QVTKOpenGLWidget* _vtkWidget;
    vtkRenderer* _renderer;

    QTimer _timer;

    ECCutD::Block3DForCutManager_ptr _blkMgr;
    ECCutD::PointForCutManager_ptr _fpMgr;
    ECCutD::PointForCutManager_ptr _lpMgr;
    ECCutD::PointForCutManager_ptr _mpMgr;
    ECCutGL::DisplayBlock3DForCutsInOneNodeJoint_ptr _vis;
};

//#endif // KBTWINDOW_H
