#include "CuttingWidget.h"

#include <QVBoxLayout>
#include <QDockWidget>
#include <QWidget>
#include <QPushButton>
#include <QSpacerItem>

#include <vtkNew.h>
#include <vtkObject.h>
#include <vtkCommand.h>
#include <vtkLookupTable.h>
#include <vtkColorTransferFunction.h>
#include <vtkAssembly.h>
#include <vtkActor.h>
#include <vtkProp3DCollection.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkInteractorObserver.h>
#include <vtkInteractorStyleFlight.h>
#include <vtkAssembly.h>
#include <vtkAssemblyPath.h>
#include <vtkAssemblyNode.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkProperty.h>
#include <vtkCellPicker.h>
#include <vtkQuad.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorObserver.h>
#include <vtkScalarBarActor.h>

#include <vtkNew.h>
#include <vtkCommand.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkInteractorStyleFlight.h>
#include <vtkRenderer.h>
#include <vtkProp3DCollection.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkTextActor.h>
#include <vtkTextRepresentation.h>
#include <vtkTextProperty.h>
#include <vtkAssembly.h>
#include <vtkAssemblyPath.h>
#include <vtkAssemblyNode.h>
#include <vtkCellPicker.h>
#include <vtkConeSource.h>
#include <vtkSphereSource.h>
#include <vtkLine.h>
#include <vtkLookupTable.h>
#include <vtkFloatArray.h>
#include <vtkCellData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyData.h>
#include <vtkPolyDataCollection.h>
#include <vtkCellArray.h>
#include <vtkQuad.h>
#include <vtkAxisActor.h>
#include <vtkArrowSource.h>
#include <vtkTransformFilter.h>
#include <vtkPropPicker.h>
#include <vtkCellPicker.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>
#include <vtkAxesActor.h>
#include <vtkPlane.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <QVTKOpenGLWidget.h>

#include "Dataset/TopologyCells/Cell3D.h"
#include "Dataset/ComputingElements/Block.h"
#include "ECUtilities/ComputingResources.h"


#include "ECVisualization/DATA/Cell3DDynamicVisualObject.h"
#include "ECVisualization/DATA/BlockViewer.h"
#include "ECVisualization/ALG/BaseVisualizationJoint.h"
#include "ECVisualization/GUI/QVTKWindow.h"

#include "ECIO/ModelObjWriter.h"

#include "ECCutDATA/Block3DForCut.h"

#include "ECCutALG/BaseConfig.h"
#include "ECCutALG/ConvertToBlock3DsJnt.h"

#include "ECCutIO/JDOMCuttingWriter_V10.h"

#include "ECCutALG/DetermineDomainWorkflow.h"
#include "ECCutALG/createDetermineDomainWorkflow.h"
#include "ECCutDExports.h"
#include "ECCutAExports.h"
#include "ECCutGExports.h"
//#include "createGeneralCuttingViewer.h"


using namespace ECU;
using namespace ECD;
using namespace ECVG;
using namespace ECCutD;
using namespace ECCutA;
//using namespace ECCutG;


// in CuttingMainWindow.cpp
//extern void initVis(GeneralCuttingViewer* viewer, ComputingResources* resources);

class BlockPickerInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
    static BlockPickerInteractorStyle* New();
    vtkTypeMacro(BlockPickerInteractorStyle, vtkInteractorStyleTrackballCamera)

    BlockPickerInteractorStyle()
        : vtkInteractorStyleTrackballCamera()
        //, Widget(nullptr)
        , BVMgr(nullptr)
        , HighlightColor(1,0,0)
        , Interactive(true)
    {
        KeyPressActivationOff();
    }
    virtual ~BlockPickerInteractorStyle() override
    {
    }
    virtual void reset()
    {
        vtkCamera* camera = GetDefaultRenderer()->GetActiveCamera();
        if(camera){
            camera->SetFocalPoint(0,0,0);
            camera->SetViewUp(0,1,0);
            camera->SetPosition(0,0,2);
        }
    }
    virtual void OnLeftButtonDown()override
    {
        if(!Interactive){
            vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
            return;
        }
        int* clickPos = this->GetInteractor()->GetEventPosition();

        // Pick from this location.
        Picker->Pick(clickPos[0], clickPos[1], 0, this->GetDefaultRenderer());

        vtkActor* a = nullptr;
        if(Picker->GetAssembly()){
            vtkAssemblyPath* path= Picker->GetPath();
            vtkAssemblyNode* node = path->GetLastNode();
            a = vtkActor::SafeDownCast(node->GetViewProp());
        }
        ///
        ///  this complex if-statement is used to decrease the time to
        /// invoke vtkActor::getProperty()->SetProperty()
        ///
        ECB::ID curIdx=ECB::MAXID, lastIdx=ECB::MAXID;
        if(nullptr!=LastPickedActor)
            lastIdx = determineSectionID(LastPickedActor);
        if(nullptr!=a){
            curIdx = determineSectionID(a);
            EC_ASSERT(curIdx!=ECB::MAXID);
        }

        if(nullptr!=a){
            if(nullptr!=LastPickedActor){
                if(a!=LastPickedActor){
                    resetLastActor();
                    highlightCurrentActor(a);
                }
                // else  "a==LastPickedActor", nothing need to be done
            } else {
                highlightCurrentActor(a);
            }
//            Widget->aSelectBlockID(curIdx);
        }
//        else {
//            if(nullptr!=LastPickedActor)
//                resetLastActor();
//        }
        vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
    }

    virtual void OnRightButtonDown()override
    {
        if(nullptr!=LastPickedActor)
            resetLastActor();
        vtkInteractorStyleTrackballCamera::OnRightButtonDown();
    }

    void resetLastActor(){
        if(LastPickedActor){
            LastPickedActor->GetProperty()->DeepCopy(LastPickedProperty);
            LastPickedActor = nullptr;
        }
    }
    void setLastActor(vtkActor* actor){
        this->LastPickedActor = actor;
        this->LastPickedProperty->DeepCopy(actor->GetProperty());
    }
    void highlightCurrentActor(vtkActor* actor){
        setLastActor(actor);
        actor->GetProperty()->SetColor(HighlightColor.data());
        actor->GetProperty()->SetDiffuse(1.0);
        actor->GetProperty()->SetSpecular(0.0);
        actor->GetProperty()->SetRepresentationToWireframe();
        printf("select block\n");
        fflush(stdout);
    }

    ECB::ID determineSectionID(vtkActor* actor){
        if(nullptr!=actor && nullptr!=BVMgr){
            for(ECB::ID i=0; i<BVMgr->getNumberOfCell3DVisualObjects(); ++i){
                if(BVMgr->getCell3DVisualObject(i)->getVTKActor()==actor)
                    return i;
            }
        }
        return ECB::MAXID;
    }


//    CuttingWidget* Widget;
    ECVD::Cell3DVisualObjectManager* BVMgr;
    ECMath::Vector3d HighlightColor;
    bool Interactive;
private:
    vtkNew<vtkCellPicker>  Picker;
    vtkSmartPointer<vtkActor>  LastPickedActor;
    vtkNew<vtkProperty> LastPickedProperty;
};

vtkStandardNewMacro(BlockPickerInteractorStyle);

static vtkNew<BlockPickerInteractorStyle> Style;


//CuttingWidget::CuttingWidget(QWidget *parent)
//  : QWidget(parent)
//  , _vtkWidget(new QVTKOpenGLWidget)
//  , _scene(nullptr)
//{
//    // determine domain
//    BaseCuttingWorkflow_ptr wf = ECCutA::createDetermineDomainWorkflow();
//    _resources = wf->getComputingResources();
//    EC_ASSERT(_resources.valid());

//    _cutViewer  = createGeneralCuttingViewer(wf, _resources);
//    initVis(_cutViewer, _resources);
//    _cutViewer->setDisplayBlocksInOneNode(false);
//    _initializeUI();
//    if(!wf->initialize()){
//        EC_ERROR_F()<<"CuttingWorkflow initialize failed";
//        return ;
//    }
//    _initializeVisualization();
//    _cutViewer->initialize(); // load joint polygons

//    // update scene
//    _timer.setInterval(40);
//    connect(&_timer, SIGNAL(timeout()), this, SLOT(render()));
//    _timer.start();

//    // set interacte style

//    Style->Widget = this;
//    Style->BVMgr = _cutViewer->getBlockViewerManager();
//    Style->SetDefaultRenderer(_renderer);
//    Style->Interactive = false;
//    vtkNew<vtkGenericOpenGLRenderWindow> renWin;
//    _vtkWidget->SetRenderWindow(renWin);
//    renWin->AddRenderer(_renderer);

//    _renderer->SetBackground2(0.529, 0.8078, 0.92157);
//    _renderer->SetBackground(1, 1, 1);
//    _renderer->SetGradientBackground(1);

//    _vtkWidget->GetInteractor()->SetInteractorStyle(Style);
//    renWin->Render();

//    return;
//}

//CuttingWidget::~CuttingWidget()
//{
//}

//void CuttingWidget::render()
//{
//    _renderer->Render();
//}

//EC_ACCESS_PROPERTY_BY_PTR_CPP_NoMDF(GeneralCuttingViewer
//         , _cutViewer,  DetemineDomainWorkflow, CuttingWidget);

//void CuttingWidget::_initializeUI()
//{
//    _scene = _vtkWidget;

//    QVBoxLayout* layout = new QVBoxLayout(this);
//    layout->addWidget(_scene);

//    auto createBtn = [](const char* str, bool enabled) {
//        QPushButton* btn = new QPushButton();
//        btn->setText(QString(str));
//        btn->setEnabled(enabled);
//        return btn;
//    };

//    {
//        char str[100];
//        for(int i=0; i<1; ++i){
//            QHBoxLayout* hlayout = new QHBoxLayout;
//            sprintf(str, "%d-Analyze",i);
//            _analyzeBtns = createBtn(str, false);
//            _allbtns[0] = _analyzeBtns;
//            hlayout->addWidget(_analyzeBtns);
//            hlayout->addSpacing(30);

//            sprintf(str, "%d-PolyInter",i);
//            _intPolyBtns = createBtn(str, false);
//            _allbtns[1] = _intPolyBtns;
//            hlayout->addWidget(_intPolyBtns);

//            sprintf(str, "%d-LineInter",i);
//            _intLinesBtns = createBtn(str, false);
//            _allbtns[2] = _intLinesBtns;
//            hlayout->addWidget(_intLinesBtns);

//            sprintf(str, "%d-newPoly",i);
//            _searchPolygonsBtns = createBtn(str, false);
//            _allbtns[3] = _searchPolygonsBtns;
//            hlayout->addWidget(_searchPolygonsBtns);

//            sprintf(str, "%d-newPolyhedron",i);
//            _searchPolyhedronsBtns = createBtn(str, false);
//            _allbtns[4] = _searchPolyhedronsBtns;
//            hlayout->addWidget(_searchPolyhedronsBtns);
//            layout->addLayout(hlayout);
//        }

//        _analyzeBtns->setEnabled(true);
//        _intPolyBtns->setEnabled(true);
//    }
//    connect(_analyzeBtns, SIGNAL(pressed()), this, SLOT(analyze1()));
//    connect(_intPolyBtns, SIGNAL(pressed()), this, SLOT(intersectPolygons1()));
//    connect(_intLinesBtns, SIGNAL(pressed()), this, SLOT(intersectLines1()));
//    connect(_searchPolygonsBtns, SIGNAL(pressed()), this, SLOT(searchPolygons1()));
//    connect(_searchPolyhedronsBtns, SIGNAL(pressed()), this, SLOT(searchPolyhedrons1()));

//    setLayout(layout);

////    setMinimumSize(800, 600);

//    resize(400,300);
//}

//void CuttingWidget::_initializeVisualization()
//{
//    _renderer->AddActor(_cutViewer->getSceneNode());
//    _cutViewer->setComputingResources(_resources);
//    EC_DEBUG()<<"initialize done";
//}

//void initMaterials(ECU::ComputingResources* crs)
//{
//    EC_ASSERT(crs);
//    EC_RESOURCE_GET_MANAGER(crs, ECD::BlockMaterialManager
//                       , CutTags::kBlockMaterialManager_Input, blkMatMgr)

//    if(blkMatMgr==nullptr) { // init block material
//        blkMatMgr = ECD::BlockMaterialManager::New();
//        ECD::BlockMaterial_ptr bm = ECD::BlockMaterial::New();
//        bm->setMaterialName("blkMatName");
//        blkMatMgr->addBlockMaterial(bm);
//        crs->setContainerManager(
//                    CutTags::kBlockMaterialManager_Input, blkMatMgr);
//    }

//    EC_RESOURCE_GET_MANAGER(crs, ECD::JointMaterialManager
//                       , CutTags::kJointMaterialManager_Input, jntMatMgr)
//    if(nullptr==jntMatMgr){
//        jntMatMgr = ECD::JointMaterialManager::New();
//        EC_RESOURCE_GET_MANAGER(crs, ECCutD::Block3DForCutManager
//                            , CutTags::kBlock3DForCutManager, blkCMgr);
//        EC_ASSERT(blkCMgr&& "CutTags::kBlock3DForCutManager not found");
//        std::set<ECB::String>  strSet;
//        ECCutD::Block3DForCut_ptr bc;
//        for(ECB::ID i=0; i<blkCMgr->getNumberOfBlock3DForCuts(); ++i){
//            bc = blkCMgr->getBlock3DForCut(i);
//            for(ECB::ID j=0; j<bc->getNumberOfFaceMaterialNames(); ++j)
//                strSet.insert(bc->getFaceMaterialName(j));
//        }
//        ECD::JointMaterial_ptr jm;
//        for(auto& jmName: strSet){
//            jm = ECD::JointMaterial::New();
//            jm->setMaterialName(jmName);
//            jntMatMgr->addJointMaterial(jm);
//        }
//        crs->setContainerManager(
//                    CutTags::kJointMaterialManager_Input, jntMatMgr);
//    }
//}

//void CuttingWidget::_triggerSubProcess(CuttingStage stage, CuttingPhase idx)
//{

//    GeneralCuttingViewer_ptr tv = _cutViewer;
//    if(!tv.valid()) {
//        EC_ERROR_F()<<ECB::MSG("workflow [%d] not set, return\n", stage);
//        return;
//    }
//    BaseCuttingWorkflow_ptr wf = tv->getCuttingWorkflow();

//    auto initBlockViewerManagerAndWriteData = [&tv, this]() {
//        Style->BVMgr = tv->getBlockViewerManager();
//        Style->Interactive=true;

//        EC_RESOURCE_GET_MANAGER(_resources, ECCutD::Block3DForCutManager
//                    , CutTags::kBlock3DForCutManager, blkCutMgr)
//        EC_ASSERT(blkCutMgr.valid() && "search polyhedron failed");
//        if(!blkCutMgr.valid())
//            return;

//        ECCutIO::JDOMCuttingWriter_V10_ptr writer
//                = ECCutIO::JDOMCuttingWriter_V10::New();
//        writer->setFileName("cutting");
//        writer->setBlock3DForCutManager(blkCutMgr);
//        writer->write();
////        ECIO::ModelObjWriter_ptr objWriter = ECIO::ModelObjWriter::New();
////		objWriter->setFileName("model");
////        objWriter->setBlock3DManager(blkMgr);
////		objWriter->write();

//        if(blkCutMgr.valid()){
//            initMaterials(_resources);
//            ConvertToBlock3DsJnt_ptr convert = ConvertToBlock3DsJnt::New();
//            convert->setComputingResources(_resources);
//            bool flag = convert->execute();
//            EC_ASSERT(flag && "convert Block3DForCut to Block3D failed");
//        }
//        EC_RESOURCE_GET_MANAGER(_resources, ECD::Block3DManager
//                                , CutTags::kBlock3DManager_Output
//                                , blkMgr)
//        EC_ASSERT(blkMgr && "convert Block3DForCut to Block3D failed");
//    };

//    switch(idx) {
//    case CuttingPhase::FullProcess:
//        wf->analyze();
//        tv->viewPolygonIntersection();
//        tv->viewLineReintersection();
//        tv->viewSearchedPolygons();
//        tv->viewSearchedPolyhedrons();
//        initBlockViewerManagerAndWriteData();
//        break;
//    case CuttingPhase::IntPolygons:
//        wf->intersectPolygons();
//        tv->viewPolygonIntersection();
//        break;
//    case CuttingPhase::IntLines:
//        wf->intersectLines();
//        tv->viewLineReintersection();
//        break;
//    case CuttingPhase::SearchPolygons:
//        wf->searchNewPolygons();
//        tv->viewSearchedPolygons();
//        break;
//    case CuttingPhase::SearchPolyhedrons:
//        wf->searchPolyhedrons();
//        tv->viewSearchedPolyhedrons();
//        initBlockViewerManagerAndWriteData();
//        break;
//    }
//}

//void CuttingWidget::_handleBtns(CuttingStage stage, CuttingPhase idx)
//{
//    if(idx==CuttingPhase::FullProcess) { // analyze
//        _allbtns[CuttingPhase::IntPolygons]->setEnabled(false);
//    } else if(idx==CuttingPhase::IntPolygons) {
//        _allbtns[CuttingPhase::FullProcess]->setEnabled(false);// analyze btn
//        _allbtns[CuttingPhase::IntPolygons]->setEnabled(false);
//        _allbtns[CuttingPhase::IntLines   ]->setEnabled(true);
//    } else if(idx==CuttingPhase::IntLines || idx==CuttingPhase::SearchPolygons) {
//        _allbtns[idx  ]->setEnabled(false);
//        _allbtns[idx+1]->setEnabled(true);
//    } else if(idx==CuttingPhase::SearchPolyhedrons) {
//        _allbtns[CuttingPhase::SearchPolyhedrons]->setEnabled(false);
//    }
//    _triggerSubProcess(stage, idx);
//}


//void CuttingWidget::analyze1()
//{
//    std::cout<<"analyze1"<<std::endl;
//    _handleBtns(CuttingStage::DeterminDomain, CuttingPhase::FullProcess);
//}

//void CuttingWidget::intersectPolygons1()
//{
//    std::cout<<"intersectPolygons1"<<std::endl;
//    _handleBtns(CuttingStage::DeterminDomain, CuttingPhase::IntPolygons);
//}

//void CuttingWidget::intersectLines1()
//{
//    _handleBtns(CuttingStage::DeterminDomain, CuttingPhase::IntLines);
//    std::cout<<"intersectLines1"<<std::endl;
//}

//void CuttingWidget::searchPolygons1()
//{
//    std::cout<<"searchPolygons1"<<std::endl;
//    _handleBtns(CuttingStage::DeterminDomain, CuttingPhase::SearchPolygons);
//}

//void CuttingWidget::searchPolyhedrons1()
//{
//    std::cout<<"searchPolyhedrons1"<<std::endl;
//    _handleBtns(CuttingStage::DeterminDomain, CuttingPhase::SearchPolyhedrons);
//}

//void CuttingWidget::aSelectBlockID(ECB::ID idx)
//{
//    blockSelected(idx); // emit signal
//}

//ECU::ComputingResources* CuttingWidget::getComputingResources()
//{
//    return _resources;
//}
