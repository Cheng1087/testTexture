#ifndef BlockPickerInteractorStyle_h
#define BlockPickerInteractorStyle_h

//#include <QVBoxLayout>
//#include <QDockWidget>
//#include <QWidget>
//#include <QPushButton>
//#include <QSpacerItem>

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

//    ECB::ID determineSectionID(vtkActor* actor){
//        if(nullptr!=actor && nullptr!=BVMgr){
//            for(ECB::ID i=0; i<BVMgr->getNumberOfBlock3DForCuts(); ++i){
//                if(BVMgr->getBlock3DForCut(i)->getVTKActor()==actor)
//                    return i;
//            }
//        }
//        return ECB::MAXID;
//    }

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
//    ECCutD::Block3DForCutManager_ptr BVMgr;

    ECMath::Vector3d HighlightColor;
    bool Interactive;
private:
    vtkNew<vtkCellPicker>  Picker;
    vtkSmartPointer<vtkActor>  LastPickedActor;
    vtkNew<vtkProperty> LastPickedProperty;
};

vtkStandardNewMacro(BlockPickerInteractorStyle);

//static vtkNew<BlockPickerInteractorStyle> Style;

#endif
