//
//  dxfout.c
//  jmri2dxf
//
//  Created by Daniel BRAUN on 17/01/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#include <stdio.h>
#include "dxfout.h"

// https://hackaday.io/project/165083-a-simple-dxf-output-library



void DXF_Begin(FILE *F1) {
  fprintf(F1,"999\n");fprintf(F1,"AlanCooper@StartMail.com\n");
  fprintf(F1,"0\n");  fprintf(F1,"SECTION\n");
  fprintf(F1,"2\n");  fprintf(F1,"ENTITIES\n");
}

void DXF_Point(FILE *F1,char *Layer,int Colour,double X,double Y,double Z) {
  fprintf(F1,"0\n");  fprintf(F1,"POINT\n");            // Point
  fprintf(F1,"8\n");  fprintf(F1,"%s\n",Layer);         // Layer
  fprintf(F1,"62\n"); fprintf(F1,"%d\n",Colour);        // Colour
  fprintf(F1,"10\n"); fprintf(F1,"%0.4lf\n",X);         // X
  fprintf(F1,"20\n"); fprintf(F1,"%0.4lf\n",Y);         // Y
  fprintf(F1,"30\n"); fprintf(F1,"%0.4lf\n",Z);         // Z
}

void DXF_Circle(FILE *F1,char *Layer,int Colour,double X,double Y,double Z,double R) {
  fprintf(F1,"0\n");  fprintf(F1,"CIRCLE\n");           // Circle
  fprintf(F1,"8\n");  fprintf(F1,"%s\n",Layer);         // Layer
  fprintf(F1,"62\n"); fprintf(F1,"%d\n",Colour);        // Colour
  fprintf(F1,"10\n"); fprintf(F1,"%0.4lf\n",X);         // X
  fprintf(F1,"20\n"); fprintf(F1,"%0.4lf\n",Y);         // Y
  fprintf(F1,"30\n"); fprintf(F1,"%0.4lf\n",Z);         // Z
  fprintf(F1,"40\n"); fprintf(F1,"%0.4lf\n",R);         // R
}

void DXF_Line(FILE *F1,char *Layer,int Colour,double X0,double Y0,double Z0,double X1,double Y1,double Z1) {
  fprintf(F1,"0\n");  fprintf(F1,"LINE\n");            // Line
  fprintf(F1,"8\n");  fprintf(F1,"%s\n",Layer);        // Layer
  fprintf(F1,"62\n"); fprintf(F1,"%d\n",Colour);       // Colour
  fprintf(F1,"10\n"); fprintf(F1,"%0.4lf\n",X0);       // X0
  fprintf(F1,"20\n"); fprintf(F1,"%0.4lf\n",Y0);       // Y0
  fprintf(F1,"30\n"); fprintf(F1,"%0.4lf\n",Z0);       // Z0
  fprintf(F1,"11\n"); fprintf(F1,"%0.4lf\n",X1);       // X1
  fprintf(F1,"21\n"); fprintf(F1,"%0.4lf\n",Y1);       // Y1
  fprintf(F1,"31\n"); fprintf(F1,"%0.4lf\n",Z1);       // Z1
}
void DXF_Arc(FILE *F1, char *Layer, int Colour, double CX, double CY, double CZ, double R, double ANG0, double ANG1)
{
    // DB20210118 added arc
    // https://documentation.help/AutoCAD-DXF/WS1a9193826455f5ff18cb41610ec0a2e719-7a35.htm
    fprintf(F1,"0\n");  fprintf(F1,"ARC\n");            // Arc
    //fprintf(F1,"8\n");  fprintf(F1,"%s\n",Layer);        // Layer
    //fprintf(F1,"62\n"); fprintf(F1,"%d\n",Colour);       // Colour
    fprintf(F1,"5\n"); fprintf(F1,"120\n");             // ???
    fprintf(F1,"100\n"); fprintf(F1, "AcDbEntity\n");
    fprintf(F1,"8\n");  fprintf(F1,"%s\n",Layer);        // Layer
    fprintf(F1,"62\n"); fprintf(F1,"%d\n",Colour);       // Colour
    fprintf(F1,"100\n"); fprintf(F1, "AcDbCircle\n");
    fprintf(F1,"10\n"); fprintf(F1,"%0.4lf\n",CX);       // X0
    fprintf(F1,"20\n"); fprintf(F1,"%0.4lf\n",CY);       // Y0
    fprintf(F1,"30\n"); fprintf(F1,"%0.4lf\n",CZ);       // Z0
    fprintf(F1,"40\n"); fprintf(F1,"%0.4lf\n",R);       // Z0
    fprintf(F1,"100\n"); fprintf(F1, "AcDbArc\n");
    fprintf(F1,"50\n"); fprintf(F1,"%0.4lf\n",ANG0);       // angle 0
    fprintf(F1,"51\n"); fprintf(F1,"%0.4lf\n",ANG1);       // angle 1
    //fprintf(F1,"210\n"); fprintf(F1," 0, 0, -1\n");       // Extrusion direction//
}

void DXF_PolylineBegin(FILE *F1,char *Layer,int Colour) {
  fprintf(F1,"0\n");  fprintf(F1,"POLYLINE\n");        // Open polyline
  fprintf(F1,"8\n");  fprintf(F1,"%s\n",Layer);        // Layer
  fprintf(F1,"62\n"); fprintf(F1,"%d\n",Colour);       // Colour
  fprintf(F1,"70\n"); fprintf(F1,"8\n");               // 3D POLYLINE
  fprintf(F1,"66\n"); fprintf(F1,"1\n");               // VERTICES FOLLOW
}

void DXF_PolylinePoint(FILE *F1,double X,double Y,double Z) {
  fprintf(F1,"0\n");  fprintf(F1,"VERTEX\n");          // Vertex (point)
  fprintf(F1,"70\n"); fprintf(F1,"8\n");               // 3D POLYLINE
  fprintf(F1,"10\n"); fprintf(F1,"%0.4lf\n",X);        // X
  fprintf(F1,"20\n"); fprintf(F1,"%0.4lf\n",Y);        // Y
  fprintf(F1,"30\n"); fprintf(F1,"%0.4lf\n",Z);        // Z
}

void DXF_PolylineEnd(FILE *F1) {
  fprintf(F1,"0\n"); fprintf(F1,"SEQEND\n");
}

void DXF_ShapeBegin(FILE *F1,char *Layer,int Colour) {
  Colour=Colour%16;
  fprintf(F1,"0\n");  fprintf(F1,"POLYLINE\n");        // Closed polyline
  fprintf(F1,"8\n");  fprintf(F1,"%s\n",Layer);        // Layer
  fprintf(F1,"62\n"); fprintf(F1,"%d\n",Colour);       // Colour
  fprintf(F1,"70\n"); fprintf(F1,"9\n");               // CLOSED 3D POLYLINE (SHAPE)
  fprintf(F1,"66\n"); fprintf(F1,"1\n");               // VERTICES FOLLOW
}

void DXF_ShapePoint(FILE *F1,double X,double Y,double Z) {
  fprintf(F1,"0\n");  fprintf(F1,"VERTEX\n");          // Vertex (point)
  fprintf(F1,"70\n"); fprintf(F1,"9\n");               // CLOSED 3D POLYLINE (SHAPE)
  fprintf(F1,"10\n"); fprintf(F1,"%0.4lf\n",X);        // X
  fprintf(F1,"20\n"); fprintf(F1,"%0.4lf\n",Y);        // Y
  fprintf(F1,"30\n"); fprintf(F1,"%0.4lf\n",Z);        // Z
}

void DXF_ShapeEnd(FILE *F1) {
  fprintf(F1,"0\n"); fprintf(F1,"SEQEND\n");
}


void DXF_End(FILE *F1) {
  fprintf(F1,"0\n"); fprintf(F1,"ENDSEC\n");
  fprintf(F1,"0\n"); fprintf(F1,"EOF");
}
