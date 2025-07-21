//call:  
//magpos( (*p).hal_val[it], (*p).numpos[it], (*p).step[it], (*p).bval[it], (*p).dbds[it] );
//or 
//magpos( (*p).hal_val[it], (*p).numpos[it], (*p).uhr[it],  (*p).bval[it], (*p).dbds[it] );



int magpos(ba, nm, st, b, db)
 float ba[4], st[AMCnhall], b[AMCnhall][4], db[AMCnhall][4];
 int nm;
{
 int i, i0, k, k0, l3, io, iu, id, ipos;
 float chi, chimin, dymax, alfi, alfu, xxx;

/*
ba      measured values (4)
nm      filling length of the following 3 arrays
st[]    motor steps; st[i] corresponds to b[i][0 to 3]
b[][]   calibration: values of the 4 probes
db[][]  derivative of b
*/



for(k=0;k<4;k++) if( (ba[k] < 200) || (ba[k] > 1000) ) { return(11110); }





//look for best position: 
      chimin=100000. ;
      for(i=0;i<nm;i++) {
       chi=(ba[0]-b[i][0])*(ba[0]-b[i][0])
          +(ba[1]-b[i][1])*(ba[1]-b[i][1])
          +(ba[2]-b[i][2])*(ba[2]-b[i][2])
          +(ba[3]-b[i][3])*(ba[3]-b[i][3]) ;
       if(chi < chimin) { chimin=chi; i0=i; }
      }

//
//which sensor has here the biggest change ?
      dymax=-10000.;
      k0=0;
      for(k=0;k<4;k++) {
       if(fabs(db[i0][k]) > dymax) { dymax=fabs(db[i0][k]); k0=k; }
      }
      xxx=st[i0];

//now interpolation
      alfi=9999.;
      if(i0 > 1 ) {
       id=1;
       if(st[i0-id] == st[i0]) id=2;         //some points measured twice when calibrating
       alfi=(ba[k0]-b[i0-id][k0]) / (b[i0][k0]-b[i0-id][k0]);
       xxx=st[i0-id]*(1.-alfi)+st[i0]*alfi;
      }

      if(i0 < nm-2 ) {
       id=1;
       if(st[i0+id] == st[i0]) id=2;
       alfu=(ba[k0]-b[i0][k0]) / (b[i0+id][k0]-b[i0][k0]);
       if(fabs(alfu) < fabs(alfi)) xxx=st[i0]*(1.-alfu)+st[i0+id]*alfu;
      }

      ipos=xxx;
      if( (xxx-st[i0] > 50.) || (st[i0]-xxx > 50.) ) ipos=st[i0];
      if(ipos> 6000) ipos=st[i0];
      if(ipos<-6000) ipos=st[i0];
      return(ipos);

}
