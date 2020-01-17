//#ifndef ECCUTGUI_JOINTSWIDGET_H
//#define ECCUTGUI_JOINTSWIDGET_H

//#include "ECCutGUI/BaseConfig.h"
//#include "Base/IDList.h"
//#include "vtkNew.h"
//#include <QTimer>
//#include <QWidget>
////#include "GeneralCuttingViewer.h"

//class vtkRenderer;

//class QPushButton;
//class QVTKOpenGLWidget;

//namespace ECU {
//class ComputingResources;
//}
//namespace ECD {
//class Block3D;
//}
//namespace ECVG
//{
//class QVTKWindow;
//}

//namespace ECCutG
//{
//class ECCutGExports CuttingWidget : public QWidget
//{
//    Q_OBJECT

//public:
//    explicit CuttingWidget(QWidget *parent = nullptr);
//    virtual ~CuttingWidget() ;

//public:
//    //EC_ACCESS_PROPERTY_BY_PTR_HEADER(GeneralCuttingViewer, DetemineDomainWorkflow)
//    //EC_ACCESS_PROPERTY_BY_PTR_HEADER(GeneralCuttingViewer, FullCuttingWorkflow)

//signals:
//    void selectBlock();
//    void blockSelected(ECB::ID blkIdx);
////    void blockSelected(ECD::Block3D* blk);

//public:
//    void aSelectBlockID(ECB::ID idx);
//    ECU::ComputingResources* getComputingResources();

//protected slots:
//    void analyze1();
//    void intersectPolygons1();
//    void intersectLines1();
//    void searchPolygons1();
//    void searchPolyhedrons1();

//    void render();

//protected:
//    void _initializeVisualization();
//    void _initializeUI();

//    enum CuttingStage {DeterminDomain=0, FullCut=1, CutConcave=2} ;
//    enum CuttingPhase {FullProcess=0, IntPolygons=1, IntLines=2
//                       , SearchPolygons=3, SearchPolyhedrons=4} ;
//    void _handleBtns(CuttingStage stage, CuttingPhase idx);
//    void _triggerSubProcess(CuttingStage stage, CuttingPhase idx);

//private:
////    ECVG::QVTKWindow* _window;
//    QVTKOpenGLWidget* _vtkWidget;
//    vtkNew<vtkRenderer> _renderer;
//    QWidget* _scene; // contain @_window as content
//    //ECB::SmartPtr<GeneralCuttingViewer> _cutViewer;
//    ECB::SmartPtr<ECU::ComputingResources> _resources;
//    QPushButton* _analyzeBtns;
//    QPushButton* _intPolyBtns;
//    QPushButton* _intLinesBtns;
//    QPushButton* _searchPolygonsBtns;
//    QPushButton* _searchPolyhedronsBtns;
//    QPushButton* _allbtns[5];
//    QTimer _timer;
//};
//}
//#endif // JOINTSMAINWINDOW_H
