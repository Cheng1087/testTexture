#include "vtkTextureMapToECObject.h"

#include "vtkCellData.h"
#include "vtkDataSet.h"
#include "vtkFloatArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkCell.h"
vtkStandardNewMacro(vtkTextureMapToECObject);

vtkTextureMapToECObject::vtkTextureMapToECObject()
{
    //all zero
    this->Center[0] = this->Center[1] = this->Center[2] = 0.0;
    this->Normal[0] = this->Normal[1] = this->Normal[2] = 0.0;
    this->MAX[0] = this->MAX[1] = this->MAX[2] = 0.0;
    this->MIN[0] = this->MIN[1] = this->MIN[2] = 0.0;
}
int vtkTextureMapToECObject::RequestData(
        vtkInformation *vtkNotUsed(request),
        vtkInformationVector **inputVector,
        vtkInformationVector *outputVector)
{
    // get the info objects
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    // get the input and output
    vtkDataSet *input = vtkDataSet::SafeDownCast(
                inInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkDataSet *output = vtkDataSet::SafeDownCast(
                outInfo->Get(vtkDataObject::DATA_OBJECT()));

    vtkIdType numPts = input->GetNumberOfPoints();

    vtkFloatArray *newTCoords;
    vtkIdType ptId;
    double p[3];
    double tcoords[2];
    //for getpoint
    // First, copy the input to the output as a starting point
    output->CopyStructure( input );
    if ( numPts < 1 )
    {
        vtkErrorMacro(<<"Can't generate texture coordinates without points");
        return 1;
    }
    if ( 1 )
    {
        this->Center[0] = this->Center[1] = this->Center[2] = 0.0;
        for ( ptId=0; ptId < numPts; ptId++ )
        {
            input->GetPoint(ptId, p);
            this->Center[0] += p[0];
            this->Center[1] += p[1];
            this->Center[2] += p[2];
        }
        this->Center[0] /= numPts;
        this->Center[1] /= numPts;
        this->Center[2] /= numPts;

        vtkDebugMacro(<<"Center computed as: (" << this->Center[0] <<", "
                                                << this->Center[1] <<", "
                                                << this->Center[2] <<")");
    }
    newTCoords = vtkFloatArray::New();
    newTCoords->SetName("Texture Coordinates");
    newTCoords->SetNumberOfComponents(2);
    newTCoords->SetNumberOfTuples(numPts);
    double bounds[6],m2M[3],edge[3];
    double temp,width,height;
    input->GetBounds(bounds);
    temp = 0;
    //edge here is to find the max edge,
    //the purpose is to define the cube
    edge[0] = bounds[1]-bounds[0];
    edge[1] = bounds[3]-bounds[2];
    edge[2] = bounds[5]-bounds[4];
    if(edge[0]>edge[1])
    {
        temp = edge[0];
    }else {
        temp = edge[1];
    }
    if(temp <edge[2])
    {
        temp = edge[2];
    }
    for (int i=0;i<3;i++)
    {
        this->MAX[i] = bounds[2*i+1];
        this->MIN[i] = bounds[2*i];
        m2M[i] = this->MAX[i] - this->MIN[i];
    }
    width = edge[0]+2*edge[2];
    height = edge[1]+2*edge[2];
    double disM[3],dism[3],planePt[3],planeNor[3],project[3];
    //    double count =0;
    //start calculate textures coordinate
    for ( ptId=0; ptId < numPts; ptId++ )
    {
        output->GetPoint(ptId,p);
        vtkMath::Subtract(p,this->MAX,disM);
        vtkMath::Subtract(p,this->MIN,dism);

        vtkDebugMacro(<<"point "<<ptId<<endl);
        vtkDebugMacro(<<"point "<<p[0]<<" , "<<p[1]<<" , "<<p[2]<<endl);

        //find the closer point from MAX and MIN
        GetPlanePointAndNormal(p,planePt,planeNor);

        vtkDebugMacro(<<"plane point "<<planePt[0]<<" , "<<planePt[1]<<" , "<<planePt[2]<<endl);
        vtkDebugMacro(<<"plane normal "<<planeNor[0]<<" , "<<planeNor[1]<<" , "<<planeNor[2]<<endl);
        GetIntersectWithLineAndPlane(p,planePt,planeNor,project);
        vtkDebugMacro(<<"project "<<project[0]<<" , "<<project[1]<<" , "<<project[2]<<endl);

        CalculateTcoord(project,planeNor,edge,tcoords);


        newTCoords->SetTuple(ptId,tcoords);
    }
    // Update ourselves
    output->GetPointData()->CopyTCoordsOff();
    output->GetPointData()->PassData(input->GetPointData());
    output->GetCellData()->PassData(input->GetCellData());

    output->GetPointData()->SetTCoords(newTCoords);
    newTCoords->Delete();
    return 1;
}

void vtkTextureMapToECObject::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);

    os << indent << "Center: (" << this->Center[0] << ", "
       << this->Center[1] << ", " << this->Center[2] << " )\n";

}
/**
 * @brief vtkTextureMapToECObject::GetIntersectWithLineAndPlane
 * @param point
 * @param direct
 * @param planePoint
 * @param planeNormal
 * @param project is the projection on the plane
 */
void vtkTextureMapToECObject::GetIntersectWithLineAndPlane(double point[3],
                                                            double planePoint[3],
                                                            double planeNormal[3],
                                                            double project[3])
{
    double distance[3],direct[3];
    vtkMath::Subtract(planePoint,point,distance);
    vtkMath::Subtract(point, this->Center, direct);
    double d = vtkMath::Dot(distance,planeNormal)/vtkMath::Dot(direct,planeNormal);
    vtkMath::MultiplyScalar(direct,d);
    vtkMath::Add(direct,point,project);
}
/**
 * @brief vtkTextureMapToECObject::GetPlanePointAndNormal
 * @param point
 * @param planePoint
 * @param planeNormal
 * calculate and define a plane to use for projection
 */
void vtkTextureMapToECObject::GetPlanePointAndNormal(double point[3],
                                                    double planePoint[3],
                                                    double planeNormal[3])
{
    double dir[3];
    vtkMath::Subtract(point,this->Center,dir);
    for (int i=0;i<3;i++)
    {
        dir[i] = dir[i]/(this->MAX[i]-this->MIN[i]);
    }
    double temp;
    temp = vtkMath::Max(fabs(dir[0]),fabs(dir[1]));
    temp = vtkMath::Max(fabs(dir[2]),temp);
    if(temp == fabs(dir[2]))
    {
        planeNormal[0] = planeNormal[1] = 0;
        if(dir[2]>0)
        {
            planeNormal[2] = 1;
            planePoint[0] = this->MAX[0];
            planePoint[1] = this->MAX[1];
            planePoint[2] = this->MAX[2];
        }else {
            planeNormal[2] = -1;
            planePoint[0] = this->MIN[0];
            planePoint[1] = this->MIN[1];
            planePoint[2] = this->MIN[2];
        }
    }
    if(temp == fabs(dir[1]))
    {
        planeNormal[0] = planeNormal[2] = 0;
        if(dir[1]>0)
        {
            planeNormal[1] = 1;
            planePoint[0] = this->MAX[0];
            planePoint[1] = this->MAX[1];
            planePoint[2] = this->MAX[2];
        }else {
            planeNormal[1] = -1;
            planePoint[0] = this->MIN[0];
            planePoint[1] = this->MIN[1];
            planePoint[2] = this->MIN[2];
        }
    }
    if(temp == fabs(dir[0]))
    {
        planeNormal[1] = planeNormal[2] = 0;
        if(dir[0]>0)
        {
            planeNormal[0] = 1;
            planePoint[0] = this->MAX[0];
            planePoint[1] = this->MAX[1];
            planePoint[2] = this->MAX[2];
        }else {
            planeNormal[0] = -1;
            planePoint[0] = this->MIN[0];
            planePoint[1] = this->MIN[1];
            planePoint[2] = this->MIN[2];
        }
    }
}
/**
 * @brief vtkTextureMapToECObject::CalculateTcoord
 * @param project
 * @param planeNor
 * @param edge is the Side length of the bounding box
 * @param tcoords
 * calculate tcoords by projection point
 */
void vtkTextureMapToECObject::CalculateTcoord(double project[3],double planeNor[3],double edge[3],double tcoords[2])
{
    double width,height,w,h;
    width = edge[0]+2*edge[2];
    height = edge[1]+2*edge[2];
    double trans = 0,increment=0;
    //face +x
    if(planeNor[0] == 1)
    {
        w = this->MAX[2] - project[2];
        h = project[1] - this->MIN[1];
        tcoords[0] = (w+edge[0]+edge[2])/width;
        tcoords[1] = h/edge[1];
        increment = (project[2] - this->MIN[2])/height;
        trans = 1-2*increment;
        tcoords[1] = trans*tcoords[1]+increment;
    }
    //face -x
    if(planeNor[0] == -1)
    {
        w = project[2] - this->MIN[2];
        h = project[1] - this->MIN[1];
        tcoords[0] = w/width;
        tcoords[1] = h/edge[1];
        increment = (project[2] - this->MIN[2])/height;
        trans = 1-2*increment;
        tcoords[1] = trans*tcoords[1]+increment;
    }
    //face +y
    if(planeNor[1] == 1)
    {
        w = project[0] - this->MIN[0];
        h = this->MAX[2] - project[2];
        tcoords[0] = w/edge[0];
        tcoords[1] = (h + edge[1] + edge[2])/height;
        increment = (project[2] - this->MIN[2])/height;
        trans = 1-2*increment;
        tcoords[0] = trans*tcoords[0]+increment;
    }
    //face -y
    if(planeNor[1] == -1)
    {
        w = project[0] - this->MIN[0];
        h = project[2] - this->MIN[2];
        tcoords[0] = w/edge[0];
        tcoords[1] = h/height;
        increment = (project[2] - this->MIN[2])/height;
        trans = 1-2*increment;
        tcoords[0] = trans*tcoords[0]+increment;
    }
    //face +z
    if(planeNor[2] == 1)
    {
        w = project[0] - this->MIN[0];
        h = project[1] - this->MIN[1];
        tcoords[0] = (w+edge[2])/width;
        tcoords[1] = (h+edge[2])/height;
    }
    //face -z
    if(planeNor[2] == -1)
    {
        w = project[0] - this->MIN[0];
        h = project[1] - this->MIN[1];
        tcoords[0] = w/edge[0];
        tcoords[1] = h/edge[1];
    }
}

#define VTK_TOLERANCE 1.0e-03

void vtkTextureMapToECObject::ComputeNormal(vtkDataSet *output)
{
  vtkIdType numPts=output->GetNumberOfPoints();
  double m[9], v[3], x[3];
  vtkIdType ptId;
  int dir = 0, i;
  double length, w, *c1, *c2, *c3, det;

  //  First thing to do is to get an initial normal and point to define
  //  the plane.  Then, use this information to construct better
  //  matrices.  If problem occurs, then the point and plane becomes the
  //  fallback value.
  //
  //  Get minimum width of bounding box.
  const double *bounds = output->GetBounds();
  length = output->GetLength();

  for (w=length, i=0; i<3; i++)
  {
    this->Normal[i] = 0.0;
    if ( (bounds[2*i+1] - bounds[2*i]) < w )
    {
      dir = i;
      w = bounds[2*i+1] - bounds[2*i];
    }
  }

  //  If the bounds is perpendicular to one of the axes, then can
  //  quickly compute normal.
  //
  this->Normal[dir] = 1.0;
  if ( w <= (length*VTK_TOLERANCE) )
  {
    return;
  }

  //  Need to compute least squares approximation.  Depending on major
  //  normal direction (dir), construct matrices appropriately.
  //
  //  Compute 3x3 least squares matrix
  v[0] = v[1] = v[2] = 0.0;
  for (i=0; i<9; i++)
  {
    m[i] = 0.0;
  }

  for (ptId=0; ptId < numPts; ptId++)
  {
    output->GetPoint(ptId, x);

    v[0] += x[0]*x[2];
    v[1] += x[1]*x[2];
    v[2] += x[2];

    m[0] += x[0]*x[0];
    m[1] += x[0]*x[1];
    m[2] += x[0];

    m[3] += x[0]*x[1];
    m[4] += x[1]*x[1];
    m[5] += x[1];

    m[6] += x[0];
    m[7] += x[1];
  }
  m[8] = numPts;

  //  Solve linear system using Kramers rule
  //
  c1 = m; c2 = m+3; c3 = m+6;
  if ( (det = vtkMath::Determinant3x3 (c1,c2,c3)) <= VTK_TOLERANCE )
  {
    return;
  }

  this->Normal[0] = vtkMath::Determinant3x3 (v,c2,c3) / det;
  this->Normal[1] = vtkMath::Determinant3x3 (c1,v,c3) / det;
  this->Normal[2] = -1.0; // because of the formulation
}

void vtkTextureMapToECObject::ComputeCenter(vtkDataSet *output)
{
    double p[3];
    vtkIdType numPts = output->GetNumberOfPoints();
    for (vtkIdType ptId=0; ptId < numPts; ptId++ )
    {
        output->GetPoint(ptId, p);
        this->Center[0] += p[0];
        this->Center[1] += p[1];
        this->Center[2] += p[2];
    }
    this->Center[0] /= numPts;
    this->Center[1] /= numPts;
    this->Center[2] /= numPts;

    vtkDebugMacro(<<"Center computed as: (" << this->Center[0] <<", "
                                            << this->Center[1] <<", "
                                            << this->Center[2] <<")");
}
