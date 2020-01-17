#ifndef vtkTextureMapToECObject_H
#define vtkTextureMapToECObject_H

#include  "vtkFiltersTextureModule.h"
#include "vtkDataSetAlgorithm.h"
#include "vtkSmartPointer.h"
#include "vtkPointSet.h"
class vtkTextureMapToECObject : public vtkDataSetAlgorithm
{
public:
    vtkTypeMacro(vtkTextureMapToECObject,vtkDataSetAlgorithm)
    void PrintSelf(ostream& os, vtkIndent indent) override;

    static vtkTextureMapToECObject *New();
    vtkSetVector3Macro(Center,double)
    vtkSetVector3Macro(MAX,double)
    vtkSetVector3Macro(MIN,double)
    vtkGetVectorMacro(Center,double,3)
    void GetIntersectWithLineAndPlane(double point[3],double planePoint[3],double planeNormal[3],double project[3]);
    void GetPlanePointAndNormal(double point[3],double planePoint[3], double planeNormal[3]);
    void CalculateTcoord(double project[3],double planeNor[3],double edge[3],double tcoords[2]);
    void ComputeNormal(vtkDataSet *output);
    void ComputeCenter(vtkDataSet *output);
protected:
    vtkTextureMapToECObject();
    ~vtkTextureMapToECObject() override {}
    int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;

    double Center[3],MAX[3],MIN[3],Normal[3];
private:
    vtkTextureMapToECObject(const vtkTextureMapToECObject&) = delete;
    void operator=(const vtkTextureMapToECObject&) = delete ;
};

#endif
