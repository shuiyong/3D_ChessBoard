/*
* CSI 4341 Computer Graphics
* Programming Assignment 6 ChessMate II the Sqeual
* Author: Yong Shui, Peter Smrcek
* Due: Dec 9th,2014
*/
#include "Mesh.h"
#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif


Mesh :: Mesh( char const *filename ) {
    ifstream infile;
    try{
        infile.open(filename);
    }catch(exception &e){
        cout<<"can't open the file "<<filename<<endl;
        exit(0);
    }
    string tmp;
    int n;
    //read vertexes
    getline(infile, tmp);
    istringstream strstream(tmp); 
    strstream>>tmp>>n;
    vlist = new vector<Vector>();
    for(int i = 0; i < n; i++){
        getline(infile, tmp);
        strstream.clear();
        strstream.str(tmp);
        Vector vc;
        strstream>>vc.x>>vc.y>>vc.z;
        vlist->push_back(vc);
    }
    //read normals
    getline(infile, tmp);
    strstream.clear();
    strstream.str(tmp);
    strstream>>tmp>>n;
    nlist = new vector<Vector>();
    for(int i = 0; i < n; i++){
        getline(infile, tmp);
        strstream.clear();
        strstream.str(tmp);
        Vector vc;
        strstream>>vc.x>>vc.y>>vc.z;
        nlist->push_back(vc);
    }
    flist = new vector<vector<pair<int,int> > > ;
    
    //read faces
    getline(infile, tmp);
    strstream.clear();
    strstream.str(tmp);
    strstream>>tmp>>n;
    for(int i = 0; i < n; i++){
        getline(infile, tmp);
        int beg = 0, end = 0, j = 0, num = 0;
        while(tmp[j] > '9' || tmp[j] < '0'){
            j++;
        }
        beg = j;
        while(tmp[j] <= '9' && tmp[j] >= '0'){
            j++;
        }
        end = j;
        num = atoi(tmp.substr(beg, end - beg).c_str());
        vector<pair<int,int> > face;
        for(int k = 0; k < num; k++){
            int vindex, nindex;
            while(tmp[j] > '9' || tmp[j] < '0'){
                j++;
            }
            beg = j;
            while(tmp[j] <= '9' && tmp[j] >= '0'){
                j++;
            }
            end = j;
            vindex = atoi(tmp.substr(beg, end - beg).c_str());
            while(tmp[j] > '9' || tmp[j] < '0'){
                j++;
            }
            beg = j;
            while(tmp[j] <= '9' && tmp[j] >= '0'){
                j++;
            }
            end = j;
            nindex = atoi(tmp.substr(beg, end - beg).c_str());
            pair<int,int> mp(vindex,nindex);
            face.push_back(mp);
        }
        flist->push_back(face);
    }
    //read tlist
    getline(infile, tmp);
    strstream.clear();
    strstream.str(tmp);
    strstream>>tmp>>n;
    tlist = new vector<pair<float,float> >();
    for(int i = 0; i < n; i++){
        getline(infile, tmp);
        strstream.clear();
        strstream.str(tmp);
        pair<float,float> mp;
        strstream>>mp.first>>mp.second;
        tlist->push_back(mp);
    }
    
    //read tlist
    getline(infile, tmp);
    strstream.clear();
    strstream.str(tmp);
    strstream>>tmp;
    tmap = new vector<vector<int> >();
    for(int i = 0; i < flist->size(); i++){
        getline(infile, tmp);
        vector<int> texture;
        int beg = 0, end = 0,j = 0,count = 0;
        while(j < tmp.size() && (tmp[j] < '0' || tmp[j] >'9')){
            j++;
        }
        beg = j;
        for(count = 0; count < flist->at(i).size(); count++){
            while(j < tmp.size() && (tmp[j] >= '0' && tmp[j] <='9')){
                j++;
            }
            end = j;
            texture.push_back(atoi(tmp.substr(beg, end - beg).c_str()));
            while(j < tmp.size() && (tmp[j] < '0' || tmp[j] >'9')){
                j++;
            }
            beg = j;
        }
        tmap->push_back(texture);
    }
    
    infile.close();
}

Mesh :: ~Mesh() {
}

void Mesh :: draw() {
    for(int i = 0; i < flist->size(); i++){
        glBegin(GL_POLYGON);
            for(int j = 0; j < flist->at(i).size(); j++){
                Vector vc = vlist->at(flist->at(i)[j].first);
                Vector vc1 = nlist->at(flist->at(i)[j].second);
                pair<float, float> mp = tlist->at(tmap->at(i)[j]);
                
                glNormal3f(vc1.x,vc1.y,vc1.z);
                glTexCoord2f(mp.first, mp.second);
                glVertex3f(vc.x, vc.y, vc.z);
            }
        glEnd();
    }
}
