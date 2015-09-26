/* Oscillo pour Spark */
/* by e-Coucou        */
// v0 juin 2015

import processing.serial.*;

Serial  comPort;
short   portIndex = 2;
int     lf = 10;          //ASCII linefeed
String  inString;
float   x,x0,y,y0,z,z0;
float   ax,ay,az;
int     abs=0;

int LARGEUR = 800;
int HAUTEUR = 500;
int[]   a_x= new int[LARGEUR+1];
int[]   a_y= new int[LARGEUR+1];
int[]   a_z= new int[LARGEUR+1];
float normal;

void setup() {
  size(LARGEUR+24,HAUTEUR);
        colorMode(RGB, 255,255,255); // fixe format couleur R G B pour fill, stroke, etc...
        fill(0,0,255); // couleur remplissage RGB
        stroke (255,100,25); // couleur pourtour RGB
        strokeWeight(1); // largeur du trait en pixels
        rectMode(CORNER); // origine rectangle coin sup gauche 
        background(0);
  for (int i=0;i<LARGEUR; a_x[i++]=height/2);
  for (int i=0;i<LARGEUR; a_y[i++]=height/2);
  for (int i=0;i<LARGEUR; a_z[i++]=height/2);
  initScreen2();
  String portNom = Serial.list()[portIndex];
  println(Serial.list());
  comPort = new Serial(this, portNom, 9600);
  comPort.clear();
  comPort.bufferUntil(lf);
}

void draw() {
  printCurve();
  fill(255);
  textSize(16);
  String message= "x=(" + (int)(ax*1000.0) +")";
  text(message, (width - 100) , height / 6 ) ;
  message= "y=(" + (int)(ay*1000.0) +")";
  text(message, (width - 100) , height / 2 ) ;
  message= "z=(" + (int)(az*1000.0) +")";
  text(message, (width - 100) , 5*height / 6 ) ;
  message= String.format("N=( %5.3f )",normal);
  text(message, (100) , height / 10 ) ;
}
void serialEvent(Serial p) {
  inString = p.readString();
  print(inString); // ok ep
  try {
    // Parse the data
    String[] dataStrings = split(inString, '#');
    for (int i = 0; i < dataStrings.length; i++) {
        String type = dataStrings[i].substring(0, 4);
        String dataval = dataStrings[i].substring(4);
    if (type.equals("DEL:")) {
      } else if (type.equals("ACC:")) {
        String data[] = split(dataval, ';');
        ax = float(data[0]); ay = float(data[1]); az = float(data[2]);
        normal = sqrt(ax*ax + ay*ay + az*az);
        a_x[abs] = (int) map(ax, -1.5, 1.5, 0, HAUTEUR);
        a_y[abs] = (int) map(ay, -1.5, 1.5, 0, HAUTEUR);
        a_z[abs++] = (int) map(az, -1.5, 1.5, 0, HAUTEUR);

        if (abs >= LARGEUR/*width*/) {
              abs = LARGEUR;
              for(i=0;i<LARGEUR;a_x[i] = a_x[++i]);
              for(i=0;i<LARGEUR;a_y[i] = a_y[++i]);
              for(i=0;i<LARGEUR;a_z[i] = a_z[++i]);
          }
      }
    }
  } catch (Exception e) {
      println(e);
      println("Caught Exception");
  }
}

void initScreen2() {
    background(0,0,0);
    stroke(100,100,100);
    line(0,height/2,width,height/2);
    line(0,height/6,width,height/6);
    line(0,5*height/6,width,5*height/6);
    stroke(60,60,60);
    line(0,height/3,width,height/3);
    line(0,2*height/3,width,2*height/3);
    stroke(40,40,40);
    line(0,height/12,width,height/12);
    line(0,3*height/12,width,3*height/12);
    line(0,5*height/12,width,5*height/12);
    line(0,7*height/12,width,7*height/12);
    line(0,9*height/12,width,9*height/12);
    line(0,11*height/12,width,11*height/12);
}
void printCurve() {
    initScreen2();
    stroke(#FFFF00);
    for (int i=0;i<LARGEUR;line(i,height-a_x[i],i+1,height-a_x[++i]));
    stroke(#FF0000);
    for (int i=0;i<LARGEUR;line(i,height-a_y[i],i+1,height-a_y[++i]));
    stroke(#0000FF);
    for (int i=0;i<LARGEUR;line(i,height-a_z[i],i+1,height-a_z[++i]));
}
void drawCurve(int y, color couleur) {
  stroke(couleur);
}
