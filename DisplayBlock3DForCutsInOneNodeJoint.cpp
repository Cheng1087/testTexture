#include "DisplayBlock3DForCutsInOneNodeJoint.h"

#include <vtkNew.h>
#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkAssembly.h>
#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolygon.h>
#include <vtkPolyDataMapper.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkProp3DCollection.h>
#include <vtkCollectionIterator.h>
#include <vtkCellData.h>
#include <vtkAppendPolyData.h>
#include <vtkImageAppend.h>
#include <vtkPointData.h>
#include <vtkTextureMapToCylinder.h>
#include <vtkTextureMapToSphere.h>
#include <vtkTextureMapToPlane.h>

#include <vtkStringArray.h>
#include <vtkTextProperty.h>
#include <vtkPointSetToLabelHierarchy.h>
#include <vtkLabelPlacementMapper.h>
#include <vtkFreeTypeLabelRenderStrategy.h>
#include <vtkIdList.h>
#include <vtkFloatArray.h>
#include <vtkLookupTable.h>
#include <vtkJPEGReader.h>
#include <vtkHardwareSelector.h>
#include "Base/ValueSet.h"

#include "ECMath/Triangulate.h"
#include "ECMath/CommonGeometryComputation.h"

#include "Dataset/ComputingElements/Block.h"
#include "Dataset/ComputingElements/ComputingElementManager.h"
#include "Dataset/TopologyCells/Cell2D.h"
#include "Dataset/TopologyCells/Cell3D.h"

#include "ECVisualization/DATA/CellVisualObject.h"
#include "ECVisualization/DATA/BlockViewer.h"

#include "ECCutDATA/Block3DForCut.h"

#include "vtkTextureMapToECObject.h"
#include "vtkECPolyDataNormals.h"

using namespace ECB;
using namespace ECMath;
using namespace ECD;
using namespace ECVD;
using namespace ECVA;

namespace ECCutGL{


extern void calculateColoursForRealTime(const ECMath::Points* ptsNew
	, const ECMath::Points* ptsOld, Vector3ds& colors);

DisplayBlock3DForCutsInOneNodeJoint::DisplayBlock3DForCutsInOneNodeJoint()
    : BaseElementsDisplayJoint()
    , _ifFixedColor(true)
    , _initiated(false)
    , _originPts(vtkPoints::New())
    , _curPts(vtkPoints::New())
    , _vtkPointActor(vtkActor::New())
    , _vtkLineActor(vtkActor::New())
    , _vtkFaceActor(vtkActor::New())
    , _points(vtkPoints::New())
    , _pointColor(vtkFloatArray::New())
    , _cellColor(vtkFloatArray::New())
    , _faceProp(_vtkFaceActor->GetProperty())
    , _opacity(_faceProp->GetOpacity())

{
    getSceneNode()->AddPart(_vtkFaceActor);
     _CVMgr = ECVD::Cell3DVisualObjectManager::New();
}
DisplayBlock3DForCutsInOneNodeJoint::~DisplayBlock3DForCutsInOneNodeJoint()
{}

EC_ACCESS_PROPERTY_SHAREDPOINTER_CPP(
        ECCutD::Block3DForCutManager, _blkMgr
        , Block3DForCutManager, DisplayBlock3DForCutsInOneNodeJoint)

void DisplayBlock3DForCutsInOneNodeJoint::setFixedColor(bool ifFixColor )
{
    if(_ifFixedColor == ifFixColor)
        return;

    _ifFixedColor = ifFixColor;
    if(ifFixColor)
        _vtkFaceActor->GetMapper()->SetScalarModeToUseCellData();
    else
        _vtkFaceActor->GetMapper()->SetScalarModeToUsePointData();
}

bool DisplayBlock3DForCutsInOneNodeJoint::execute() {
	if (!_initiated) {
		_buildScene();
		_initiated = true;
	}
	else
		_updateScene();
	return true;
}
bool DisplayBlock3DForCutsInOneNodeJoint::check() const{
    return 1;
}

bool collectPoints(const ECCutD::Block3DForCutManager* blkMgr
                  , vtkPoints* pts
                  , std::vector<size_t>& pointCnts)
{
    EC_ASSERT(blkMgr && pts);
    size_t size = blkMgr->getNumberOfBlock3DForCuts();
    const Cell3D* ph=nullptr;
    const Points* tmpPts=nullptr;
    pts->Reset();
    size_t base =0;
    pointCnts.resize(size+1);
    pointCnts[0]=0;
    for(ID k=0; k<size; ++k){
        ph = blkMgr->getBlock3DForCut(k)->getCell3D();
        tmpPts = ph->getAllPoints();
        for(auto& tp: *tmpPts)
            pts->InsertNextPoint(tp[0], tp[1], tp[2]);
        pointCnts[k+1]= base + tmpPts->size();
        base = pointCnts[k+1];
    }
    return true;
}

bool drawPolygon(const Cell2DBase* face, vtkCellArray* vtkPolys, ID baseIdx)
{
    EC_ASSERT(face && vtkPolys);
    if(!isConcave(face->getAllPoints(), face->getIDList())){
        size_t size = face->getNumberOfSelfPoints();
        vtkSmartPointer<vtkPolygon> vtkPoly = vtkPolygon::New();
        vtkPoly->GetPointIds()->SetNumberOfIds(size);
        for (ID i=0;i<size;i++)
            vtkPoly->GetPointIds()->SetId(i,baseIdx+face->getID4Index(i));
        vtkPolys->InsertNextCell(vtkPoly);
        return true;
    } else {
        if(!triangulate(face, vtkPolys, baseIdx)){
            EC_ERROR_F()<<"triangulate failed ";
            return false;
        }
        return true;
    }
}
void DisplayBlock3DForCutsInOneNodeJoint::_collectMaterialName(const ECCutD::Block3DForCutManager* blkMgr)
{
    EC_ASSERT( blkMgr );
    size_t size = blkMgr->getNumberOfBlock3DForCuts();
    //add first material name
    ECB::String curName = blkMgr->getBlock3DForCut(0)->getFaceMaterialName(0);
    curName = curName.substr(0,5);
    vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();
    polydata->SetPolys(polys);
    _mapMaterialAndPolydata.insert(std::map<ECB::String,vtkSmartPointer<vtkPolyData> >::value_type(curName,polydata));
    size_t faceSize = 0;
    for(ID i=0; i<size; ++i){
        faceSize = blkMgr->getBlock3DForCut(i)->getCell3D()->getNumberOfFaces();
        for(ID j=0;j<faceSize;++j){
            curName = blkMgr->getBlock3DForCut(i)->getFaceMaterialName(j);
            curName = curName.substr(0,5);
            if(findPolydataByName(curName) == nullptr){
                vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
                vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();
                polydata->SetPolys(polys);
                _mapMaterialAndPolydata.insert(std::map<ECB::String,vtkSmartPointer<vtkPolyData> >::value_type(curName,polydata));
            }
        }
    }
    int i=0;
for ( auto iter = _mapMaterialAndPolydata.begin();iter != _mapMaterialAndPolydata.end();++iter) {
    cout<<"material name "<<i++<<" "<<iter->first<<endl;
}
}

vtkPolyData* DisplayBlock3DForCutsInOneNodeJoint::findPolydataByName(ECB::String materialName)
{
    auto iter = _mapMaterialAndPolydata.find(materialName);
    if(iter != _mapMaterialAndPolydata.end())
    {
        return iter->second;
    }else {
        return nullptr;
    }
}
bool collectTopoInfo(const ECCutD::Block3DForCutManager* blkMgr
                     , vtkCellArray* vtkPolys)
{
    EC_ASSERT(blkMgr && vtkPolys);
    size_t size = blkMgr->getNumberOfBlock3DForCuts();
    const Cell3D* ph=nullptr;
    const Cell2D* face=nullptr;
    const Points* tmpPts=nullptr;
    size_t base=0;
    for(ID k=0; k<size; ++k){
        ph = blkMgr->getBlock3DForCut(k)->getCell3D();
        tmpPts = ph->getAllPoints();
        for(ID i=0; i<ph->getNumberOfFaces(); ++i){
            face = ph->getFace(i);
            if(!drawPolygon(face, vtkPolys, base))
                return false;
        }
        base += ph->getNumberOfAllPoints();
    }
    return true;
}
bool DisplayBlock3DForCutsInOneNodeJoint::_collectTopoInfo(const ECCutD::Block3DForCutManager* blkMgr)
{
    EC_ASSERT(blkMgr);
    size_t size = blkMgr->getNumberOfBlock3DForCuts();
    const Cell3D* ph=nullptr;
    const Cell2D* face=nullptr;
    const Points* tmpPts=nullptr;
    vtkCellArray* vtkPolys = nullptr;
    size_t base=0;
    ECB::String curName;
    for(ID k=0; k<size; ++k){
        ph = blkMgr->getBlock3DForCut(k)->getCell3D();
        tmpPts = ph->getAllPoints();
        for(ID i=0; i<ph->getNumberOfFaces(); ++i){
            face = ph->getFace(i);
            curName = blkMgr->getBlock3DForCut(k)->getFaceMaterialName(i);
            curName = curName.substr(0,5);
            if(findPolydataByName(curName)==nullptr){
                return false;
            }
            vtkPolys = findPolydataByName(curName)->GetPolys();
            if(!drawPolygon(face, vtkPolys, base))
                return false;
        }
        base += ph->getNumberOfAllPoints();
    }
    return true;
}
bool arrangePolyColor(const ECCutD::Block3DForCutManager* blkMgr
                     , vtkFloatArray* vtkPolyColors
                      , const size_t numColors)
{
    EC_ASSERT(blkMgr && vtkPolyColors);
    size_t size = blkMgr->getNumberOfBlock3DForCuts();
    const Cell3D* ph=nullptr;
    const Cell2D* face=nullptr;
    const Points* tmpPts=nullptr;
    ID colorIdx = 0;

    IDList_ptr tmpIds = IDList::New();
    for(ID k=0; k<size; ++k){
        ph = blkMgr->getBlock3DForCut(k)->getCell3D();
        tmpPts = ph->getAllPoints();
        for(ID i=0; i<ph->getNumberOfFaces(); ++i){
            face = ph->getFace(i);
            if(!isConcave(face->getAllPoints(), face->getIDList()))
                vtkPolyColors->InsertNextTuple1(colorIdx);
            else {
                bool flag = ECMath::Triangulate::Process(
                            face->getAllPoints()
                            , face->getNormal()
                            , face->getIDList()
                            , tmpIds);

                if(flag) {
                    const size_t n = tmpIds->size()/3;
                    for(ID j=0; j<n; ++j)
                        vtkPolyColors->InsertNextTuple1(colorIdx);
                } else
                    return false;
            }
        }
        colorIdx = (colorIdx+1)%numColors;
    }
    return true;
}

bool arrangePointColor(vtkPoints* oldPts
                       , vtkPoints* newPts
                       , vtkFloatArray* ptColor)
{
    EC_ASSERT(oldPts && newPts);
    EC_ASSERT(oldPts->GetNumberOfPoints()==newPts->GetNumberOfPoints());
    vtkIdType size = oldPts->GetNumberOfPoints();
    ptColor->SetNumberOfTuples(size);
    double max=0, tmp=0;
    double t3[3]={0,0,0};
    auto dist = [&t3](double* p1, double* p2){
        for(ID i=0; i<3; ++i)
            t3[i] = p1[i]-p2[i];
        return sqrt(t3[0]*t3[0] + t3[1]*t3[1] + t3[2]*t3[2]);
    };

    for(vtkIdType i=0; i<size; ++i){
        tmp = dist(oldPts->GetPoint(i), newPts->GetPoint(i));
        ptColor->SetTuple1(i, tmp);
        max = tmp>max? tmp: max;
    }
    return true;
}


bool updatePoints(const ECCutD::Block3DForCutManager* blkMgr
                  , vtkPoints* vtkPts
                  , const std::vector<size_t>& pointCnts)
{
    EC_ASSERT(blkMgr && vtkPts);
    EC_ASSERT(pointCnts.size()==blkMgr->getNumberOfBlock3DForCuts());

    size_t size = blkMgr->getNumberOfBlock3DForCuts();
    ID baseIdx=0, end=0;
    const Cell3D* ph=nullptr;
    const Points* tmpPts=nullptr;
    for(ID k=0; k<size; ++k)
    {
        ph = blkMgr->getBlock3DForCut(k)->getCell3D();
        baseIdx = pointCnts[k];
        end = pointCnts[k+1];
        tmpPts = ph->getAllPoints();
        for(ID i=baseIdx; i<end; ++i){
            const Point& tp = (*tmpPts)[i-baseIdx];
            vtkPts->SetPoint(i, tp[0], tp[1], tp[2]);
        }
    }
    return true;
}


void DisplayBlock3DForCutsInOneNodeJoint::_buildScene()
{
    ECCutD::Block3DForCutManager_ptr cem = getBlock3DForCutManager();

    collectPoints(cem, _originPts, _pointCnts);
    _collectMaterialName(cem);
    _collectTopoInfo(cem);
    _curPts->DeepCopy(_originPts);
    vtkNew<vtkAppendPolyData> append;
    int i=0;
    for ( auto iter = _mapMaterialAndPolydata.begin();iter != _mapMaterialAndPolydata.end();++iter,++i) {
        iter->second->SetPoints(_curPts);
        _processPointData(iter->second,i);
        append->AddInputData(iter->second);
    }
    append->Update();
    cout<<"num of polydata in append "<<append->GetTotalNumberOfInputConnections()<<endl;
    vtkSmartPointer<vtkPolyDataMapper> faceMapper
                = vtkSmartPointer<vtkPolyDataMapper>::New();
    faceMapper->SetInputConnection(append->GetOutputPort());
//    vtkSmartPointer<vtkLookupTable> table = vtkLookupTable::New();
//    table->SetNumberOfTableValues(2);
//    table->SetTableValue(0,1,0,0);
//    table->SetTableValue(1,0,0,1);
//    table->Build();
//    faceMapper->SetScalarModeToUseCellData();
//    faceMapper->SetScalarRange(0,1);
//    faceMapper->SetScalarVisibility(1);
//    faceMapper->SetLookupTable(table);
    _vtkFaceActor->SetMapper(faceMapper);
    _vtkFaceActor->GetProperty()->SetOpacity(0.5);
    _vtkFaceActor->SetTexture(_getTexture());
//origin code
    /*
//    vtkSmartPointer<vtkPolyDataMapper> faceMapper
//            = vtkSmartPointer<vtkPolyDataMapper>::New();
//    faceMapper->SetInputData(polygonPolyData);
//    //faceMapper->SetScalarRange(0,7);

//    // poly colors
//    vtkSmartPointer<vtkLookupTable> table = vtkLookupTable::New();
//    buildColorTable(table);
//    faceMapper->SetLookupTable(table);
//    vtkFloatArray *scalars = _cellColor;
//    arrangePolyColor(cem, scalars, 8);
//    polygonPolyData->GetCellData()->SetScalars(scalars);

//    // point colors
//    _curMaxDist = arrangePointColor(_originPts, _curPts, _pointColor);

//    // actor
//    _vtkFaceActor->SetMapper(faceMapper);
//    _vtkFaceActor->GetProperty()->SetRepresentationToSurface();
//    _vtkFaceActor->GetProperty()->SetEdgeVisibility(1);
//    _vtkFaceActor->GetProperty()->SetEdgeColor(1,0,1);

//    setFixedColor(isFixedColor());
*/
}


void DisplayBlock3DForCutsInOneNodeJoint::_updateScene()
{
    assert(check());
    updatePoints(getBlock3DForCutManager(), _curPts, _pointCnts);
    if(!isFixedColor()){
        _curMaxDist = arrangePointColor(_originPts, _curPts, _pointColor);
    }
}

vtkTexture* DisplayBlock3DForCutsInOneNodeJoint::_getTexture()
{
    vtkSmartPointer<vtkImageAppend> append = vtkSmartPointer<vtkImageAppend>::New();

    String path = "D:/code/testTexture/texture/texture";
    int control[] = {0,1,0,0};
    for(int i=0;i<_mapMaterialAndPolydata.size();i++){
        vtkSmartPointer<vtkJPEGReader> reader = vtkSmartPointer<vtkJPEGReader>::New();
        String file = path + std::to_string(control[i]) + ".jpg";
        reader->SetFileName(file.c_str());
        reader->Update();
        append->AddInputConnection(reader->GetOutputPort());
    }
    append->Update();
    vtkTexture* texture = vtkTexture::New();
    texture->SetInputConnection(append->GetOutputPort());
    return texture;
}
/**
 * @brief DisplayBlock3DForCutsInOneNodeJoint::_processPointData
 * @param polydata
 * @param n start from 0
 */
void DisplayBlock3DForCutsInOneNodeJoint::_processPointData(vtkSmartPointer<vtkPolyData> polydata, int n)
{
    EC_ASSERT(polydata);
    //calculate tcoords
//    vtkSmartPointer<vtkTextureMapToECObject> map = vtkSmartPointer<vtkTextureMapToECObject>::New();
//    vtkSmartPointer<vtkTextureMapToCylinder> map = vtkSmartPointer<vtkTextureMapToCylinder>::New();
    vtkSmartPointer<vtkTextureMapToSphere> map = vtkSmartPointer<vtkTextureMapToSphere>::New();
//    vtkSmartPointer<vtkTextureMapToPlane> map = vtkSmartPointer<vtkTextureMapToPlane>::New();
    map->SetInputData(polydata);
    map->Update();
    polydata->GetPointData()->SetTCoords(map->GetPolyDataOutput()->GetPointData()->GetTCoords());
    vtkIdType num = polydata->GetPointData()->GetTCoords()->GetNumberOfTuples();
    double tcs[2];
    double total = _mapMaterialAndPolydata.size();
    for (vtkIdType i = 0; i < num; i++)
    {
        polydata->GetPointData()->GetTCoords()->GetTuple(i, tcs);
        tcs[0] = tcs[0]/total + n/total;

        polydata->GetPointData()->GetTCoords()->SetTuple(i, tcs);
    }

}
void _processCellData(vtkSmartPointer<vtkPolyData> polydata, int n)
{
    EC_ASSERT(polydata);
    int control[] = {0,1,1,1,0,0};
    vtkIdType num = polydata->GetNumberOfCells();
    vtkIntArray* scalar = vtkIntArray::New();
    scalar->SetNumberOfComponents(1);
    for(int i=0;i<num;++i){
        scalar->InsertNextTuple1(control[n]);
    }
    polydata->GetCellData()->SetScalars(scalar);
}
} //ns
