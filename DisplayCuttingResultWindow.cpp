#include "DisplayCuttingResultWindow.h"

#include <QVBoxLayout>
#include "QVTKOpenGLWidget.h"

#include <vtkNew.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkAssembly.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkActorCollection.h>
#include <vtkProp3DCollection.h>
#include <vtkJPEGReader.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkSetGet.h>
#include <vtkAssemblyPath.h>
#include <vtkAssemblyNode.h>

#include <Base/FileSystem.h>
#include <Base/LoggerManager.h>
#include <Dataset/NonTopologyCells/PolygonNTSet.h>
#include <ECVisualization/DATA/Cell2DNTCompoundVisualObject.h>

#include <ECCutIO/JDOMCuttingReader_V10.h>

#include "vtkECPolyDataNormals.h"
#include "BlockPickerInteractorStyle.h"

//static vtkNew<BlockPickerInteractorStyle> Style;

DisplayCuttingResultWindow::DisplayCuttingResultWindow(QWidget *parent)
  : QWidget(parent)
  , _vtkWidget(new QVTKOpenGLWidget)
  , _renderer(vtkRenderer::New())
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(_vtkWidget);
    setLayout(layout);

    _initVTKWidget();

    _timer.setInterval(40);
    QObject::connect(&_timer, SIGNAL(timeout()), this, SLOT(render()));
}

DisplayCuttingResultWindow::~DisplayCuttingResultWindow()
{
}

void DisplayCuttingResultWindow::render()
{
    _renderer->Render();
}

void DisplayCuttingResultWindow::_initVTKWidget()
{
//    _renderer->SetBackground2(1.0, 1.0, 1.0);
    _renderer->SetBackground2(0.529, 0.8078, 0.92157);
    _renderer->SetBackground(0, 0, 0);
    _renderer->SetGradientBackground(1);

    vtkNew<vtkGenericOpenGLRenderWindow> renWin;
    renWin->AddRenderer(_renderer);
    _vtkWidget->SetRenderWindow(renWin);
    //_vtkWidget->GetInteractor()->SetInteractorStyle(Style);


    //Style->BVMgr = //_cutViewer->getBlockViewerManager();
    //Style->SetDefaultRenderer(_renderer);
    //Style->Interactive = false;

    _readData();
}

void DisplayCuttingResultWindow::_readData()
{
    ECB::String fileName("cutting.d3m");
    if(!ECB::isFile(fileName)){
        EC_ERROR()<<"file '"<<fileName<<"' not found. exit.";
        return ;
    }
    ECCutIO::JDOMCuttingReader_V10_ptr reader
            = ECCutIO::JDOMCuttingReader_V10::New();
    if(!_blkMgr.valid())
        _blkMgr = ECCutD::Block3DForCutManager::New();
    if(!_fpMgr.valid())
        _fpMgr = ECCutD::PointForCutManager::New();
    if(!_lpMgr.valid())
        _lpMgr = ECCutD::PointForCutManager::New();
    if(!_mpMgr.valid())
        _mpMgr = ECCutD::PointForCutManager::New();

    reader->setFileName(fileName);
    reader->setBlock3DForCutManager(_blkMgr);
    reader->setFixedPointManager(_fpMgr);
    reader->setLoadingPointManager(_lpMgr);
    reader->setMeasuredPointManager(_mpMgr);

    if(!reader->read()){
        EC_ERROR()<<"read file '"<<fileName<<"' failed. exit";
        return;
    };
    if(!_vis.valid())
        _vis = ECCutGL::DisplayBlock3DForCutsInOneNodeJoint::New();
    _vis->setFixedColor(true);
    _vis->setBlock3DForCutManager(_blkMgr);
    if(!_vis->execute()){
        EC_ERROR()<<"displaying Block3DForCut failed. exit";
    }
    _renderer->AddActor(_vis->getSceneNode());
//    _processTexture();
}
void DisplayCuttingResultWindow::_processTexture()
{
    double* center;
    center = _renderer->GetCenter();
    vtkSmartPointer<vtkActorCollection> col = vtkSmartPointer<vtkActorCollection>::New();
    col = _renderer->GetActors();
    if(!col->GetNumberOfItems()){
        EC_ERROR()<<"there are no actors in render.";
    }

    int num =  _vis->getSceneNode()->GetNumberOfPaths();
    cout<<"num of actor in assembly"<<num<<endl;
    for (int i=num;i>0;i--)
    {
        _vis->getSceneNode()->RemovePart(_vis->getSceneNode()->GetParts()->GetLastProp3D());
    }
    cout<<"num of actor in assembly"<<_vis->getSceneNode()->GetNumberOfPaths()<<endl;
    vtkSmartPointer<vtkJPEGReader> reader = vtkSmartPointer<vtkJPEGReader>::New();
    reader->SetFileName("D:\\meshes\\textures\\texture_0.jpg");
    vtkSmartPointer<vtkTexture> texture = vtkSmartPointer<vtkTexture>::New();
    texture->SetInputConnection(reader->GetOutputPort());

    num = col->GetNumberOfItems();
    col->InitTraversal();
    for(int i=0;i<num;++i)
    {
        vtkSmartPointer<vtkECPolyDataNormals> norms = vtkSmartPointer<vtkECPolyDataNormals>::New();
        norms->SetCenter(center);
        vtkNew<vtkPolyData> poly;

        norms->SetInputData(col->GetNextActor()->GetMapper()->GetInput());
        vtkSmartPointer<vtkOpenGLPolyDataMapper> mapper = vtkSmartPointer<vtkOpenGLPolyDataMapper>::New();
        mapper->SetInputConnection(norms->GetOutputPort());
        std::string vs = _readGlslFile("D:\\meshes\\textures\\vertex.glsl");
        mapper->SetVertexShaderCode(vs.c_str());
        std::string fs = _readGlslFile("D:\\meshes\\textures\\fragment.glsl");
        mapper->SetFragmentShaderCode(fs.c_str());
        vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
        actor->SetTexture(texture);
        _vis->getSceneNode()->AddPart(actor);
    }
}
std::string DisplayCuttingResultWindow::_readGlslFile(const char* path)
{
    std::string str;
    std::ifstream VertexShaderStream(path, std::ios::in);
    if (VertexShaderStream.is_open()) {
        std::string Line = "";
        while (getline(VertexShaderStream, Line))
            str += "\n" + Line;
        VertexShaderStream.close();
    }
    else
    {
        throw std::exception("ReadGlslFile error!!!");
    }
    return str;
}
