#ifndef DISPLAYBLOCKSINONENODEJOINT_H
#define DISPLAYBLOCKSINONENODEJOINT_H
#include "ECVisualization/ALG/BaseElementsDisplayJoint.h"
#include "ECMath/Point.h"
#include "ECMath/Matrix/Vector.h"
#include "ECVisualization/DATA/Cell3DVisualObject.h"
class vtkActor;
class vtkActor2D;
class vtkAssembly;
class vtkPoints;
class vtkIdList;
class vtkFloatArray;
class vtkProperty;
class vtkTexture;
class vtkPolyData;
namespace ECCutD {
class Block3DForCutManager;
}


namespace ECCutGL {
/**
 * This class display all blocks in one node that greately increases
 *  display speed and reduce memory cost.
 * This class maintain a flag that indicates if this class is called
 *  for the first time.
 *  - If it is the first time, this class initiate visual scene of blocks.
 *  - If it is the second time, this class update visual scene of blocks.
 *
 * @note: FixedColor must be set before running execute.
 */
class  DisplayBlock3DForCutsInOneNodeJoint
        : public ECVA::BaseElementsDisplayJoint
{
    EC_CLASS_SYSTEM_HEADER(DisplayBlock3DForCutsInOneNodeJoint)
    EC_CLASS_NEW_FUNCTION(DisplayBlock3DForCutsInOneNodeJoint)

    EC_ACCESS_PROPERTY_BY_PTR_HEADER(
            ECCutD::Block3DForCutManager, Block3DForCutManager)

    EC_GET_BOOL_PROPERTY(_ifFixedColor, FixedColor)
    void setFixedColor(bool ifFixColor );

//    EC_GET_PROPERTY_BY_PTR_HEADER(vtkActor, VTKPointActor)
//    EC_GET_PROPERTY_BY_PTR_HEADER(vtkActor, VTKLineActor)
//    EC_GET_PROPERTY_BY_PTR_HEADER(vtkActor, VTKFaceActor)
//    EC_GET_PROPERTY_BY_PTR_HEADER(vtkActor2D, VTKPointLabelActor)
//    EC_GET_PROPERTY_BY_PTR_HEADER(vtkPoints, VTKPoints)
//    EC_GET_PROPERTY_BY_PTR_HEADER(vtkFloatArray, VTKPointData)
//    EC_GET_PROPERTY_BY_PTR_HEADER(vtkFloatArray, VTKCellData)

public:
    /*virtual*/ bool execute()override;
    /*virtual*/ bool check()const override;
protected:
	void _buildScene();
	void _updateScene();
    std::string _readGlslFile(const char* path);
    vtkTexture* _getTexture();
    void _processPointData(vtkSmartPointer<vtkPolyData> polydata, int n);
    bool _collectTopoInfo(const ECCutD::Block3DForCutManager* blkMgr);
    void _collectMaterialName(const ECCutD::Block3DForCutManager* blkMgr);
    vtkPolyData* findPolydataByName(ECB::String materialName);
private:
    bool _ifFixedColor;	// default fixed color
	bool _initiated;

    ECMath::Vector3ds _colors; // keep max number of points of blocks to avoid resize
	int _numColors;  // number of points of current Cell3D object
    ECB::SmartPtr<ECCutD::Block3DForCutManager> _blkMgr;
    ECB::SmartPtr<ECVD::Cell3DVisualObjectManager> _CVMgr;

    // used to calculate colors of vertices of blocks
    vtkSmartPointer<vtkPoints> _originPts;  ///< with _pointCnts get points of Block
    vtkSmartPointer<vtkPoints> _curPts;     ///< with _pointCnts get points of Block
    std::vector<size_t> _pointCnts; ///< record number of points of each Block

    vtkSmartPointer<vtkActor> _vtkPointActor;
    vtkSmartPointer<vtkActor> _vtkLineActor;
    vtkSmartPointer<vtkActor> _vtkFaceActor;

    vtkSmartPointer<vtkPoints> _points;
    vtkSmartPointer<vtkFloatArray> _pointColor;
    vtkSmartPointer<vtkFloatArray> _cellColor;

    vtkSmartPointer<vtkProperty> _faceProp;

    double _opacity;
    double _curMaxDist;

    std::map<ECB::String,vtkSmartPointer<vtkPolyData> > _mapMaterialAndPolydata;
};
EC_DEFINE_PTR(DisplayBlock3DForCutsInOneNodeJoint);
}
#endif
