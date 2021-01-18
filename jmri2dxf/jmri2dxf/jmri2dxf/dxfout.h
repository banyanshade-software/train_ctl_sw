// https://hackaday.io/project/165083-a-simple-dxf-output-library

#ifndef DXFOUT_H_INCLUDED
#define DXFOUT_H_INCLUDED

/* Prototypes */
void DXF_Begin(FILE *F1);
void DXF_Point(FILE *F1,char *Layer,int Colour,double X,double Y,double Z);
void DXF_Circle(FILE *F1,char *Layer,int Colour,double X,double Y,double Z,double R);
void DXF_Line(FILE *F1,char *Layer,int Colour,double X0,double Y0,double Z0,double X1,double Y1,double Z1);
void DXF_PolylineBegin(FILE *F1,char *Layer,int Colour);
void DXF_PolylinePoint(FILE *F1,double X,double Y,double Z);
void DXF_PolylineEnd(FILE *F1);
void DXF_ShapeBegin(FILE *F1,char *Layer,int Colour);
void DXF_ShapePoint(FILE *F1,double X,double Y,double Z);
void DXF_ShapeEnd(FILE *F1);
void DXF_Arc(FILE *F1, char *Layer, int Colour, double CX, double CY, double CZ, double R, double ANG0, double ANG1);
void DXF_End(FILE *F1);

#endif // DXFOUT_H_INCLUDED
