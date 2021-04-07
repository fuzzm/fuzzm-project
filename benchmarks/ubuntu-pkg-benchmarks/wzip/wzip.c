/*

wzip.c v1.1 - a preprocessor for lossy data compression

Copyright (C) 1997 Andreas Franzen

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

Andreas Franzen <Andreas.Franzen@nordkom.netsurf.de>, 24 Dec 1997.

paper mail:

A. Franzen
Seewenje Str. 203
D-28237 Bremen
Germany

*/

#define sqrt05 0.707106781186547572737310929369

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void usage()
{
  fprintf(stderr,"wzip 1.1\n");
  fprintf(stderr,"wzip is a preprocessor for LOSSY data compression.\n");
  fprintf(stderr,"It strips off random noise from the input data making it\n");
  fprintf(stderr,"possible for a common loss-less compression program\n");
  fprintf(stderr,"to compress with high efficiency.\n");
  fprintf(stderr,"compression:\n");
  fprintf(stderr,"cat data.asc | wzip -c num sf > data.i24 ; gzip -9 data.i24\n");
  fprintf(stderr,"decompression:\n");
  fprintf(stderr,"gzip -d -c data.i24.gz | wzip -d num sf > data2.asc\n");
  fprintf(stderr,"wzip can also be used for denoising.\n");
  fprintf(stderr,"denoising (soft):\n");
  fprintf(stderr,"cat data.asc | wzip -dn num sf > data2.asc\n");
  fprintf(stderr,"denoising (hard):\n");
  fprintf(stderr,"cat data.asc | wzip -hdn num sf > data2.asc\n");
  fprintf(stderr,"num is the number of data points and sf a scale factor.\n");
  fprintf(stderr,"data.asc is a textfile containing num floating-point numbers.\n");
  fprintf(stderr,"This is free-software, placed under the GNU general public\n");
  fprintf(stderr,"license as published by the free-software foundation.\n");
  fprintf(stderr,"See the file copyright for details.\n");
  fprintf(stderr,"\n");
  exit(1);
}

void wtrafo(float *y,float *w,int n)
/* 
 * Transformation of vector y into wavelet space vector w by using
 * the Haar-wavelet base.
 * n is the number of data points.
 * y and w are indexed from 0 to n-1.
 */  
{
  int a,i,b1,b2;
  float *d;
  d=(float*)malloc(sizeof(float)*n);
  a=n/2;
  for(i=0;i<a;i++)
    {
      w[i]=(y[2*i]-y[2*i+1])*sqrt05;
      d[i]=(y[2*i]+y[2*i+1])*sqrt05;
    };
  b1=0;
  b2=a;
  a=a/2;
  while(a>0) 
    {
      for(i=0;i<a;i++)
        {
          w[i+b2]=(d[2*i+b1]-d[2*i+1+b1])*sqrt05;
          d[i+b2]=(d[2*i+b1]+d[2*i+1+b1])*sqrt05;
        };
      b1=b2;
      b2=b2+a;
      a=a/2;
    };
  w[b2]=d[b1];
  free(d);
}

void wbtrafo(float *w,float *y,int n)
/* 
 * Back-transformation of wavelet space vector w into vector y by using
 * the Haar-wavelet base.
 * n is the number of data points.
 * y and w are indexed from 0 to n-1.
 */  
{
  int a,i,b1,b2;
  float *d;
  d=(float*)malloc(sizeof(float)*n);
  d[n-2]=w[n-1];
  b1=n-4;
  b2=n-2;
  a=1;
  while(a<n/2)
    {
      for(i=0;i<a;i++)
        {
          d[2*i+b1]=(d[i+b2]+w[i+b2])*sqrt05;
          d[2*i+1+b1]=(d[i+b2]-w[i+b2])*sqrt05;
        };
      b2=b1;
      b1=b1-4*a;
      a=2*a;
    };
  for(i=0;i<a;i++)
    {
      y[2*i]=(d[i]+w[i])*sqrt05;
      y[2*i+1]=(d[i]-w[i])*sqrt05;
    };
  free(d);
}

void simpledenoise(float *y, int n, float sf, int t)
{
/*
 * simple denoising of vector y by transformation into wavelet space,
 * shrinking of the wavelet coefficients towards zero, and
 * re-transformation.
 *
 * t=0, soft denoising, wavelet shrinkage
 * t=1, hard denoising, hard thresholding
 */
  float *w;
  int i;
  w=(float*)malloc(n*sizeof(float));
  wtrafo(y,w,n);
  if(sf < 0.0)
    sf = -sf;
  for (i=0;i<n-1;i++)
/*
 * The last coefficient w[n-1] remains unaffected because this is essentially
 * the arithmetic mean of the input vector.
 */
    {
/*
 * All wavelet coefficients with an absolute value smaller then the 
 * scale factor are set to zero.
 */
      if ((w[i] < sf)&&(w[i] > -sf)) 
        w[i]=0.0;
/*
 * All other coefficients move the amount of the scale factor towards zero
 * with t=0 or are untouched with t=1.
 */        
      else
        {
          if (t==0)
            {
              if (w[i] > 0.0) 
                w[i]=w[i]-sf;
              else  
                w[i]=w[i]+sf;
            }
        }
    }    
  wbtrafo(w,y,n);
  free(w);
}

void tidenoise(float *y, int n, float sf, int t)
/*
 * translation invariant denoising (could be improved)
 *
 * The simple denoising introduces artifacts into the data. The
 * structure of the wavelet base gets visible. By doing the denoising
 * process with each cyclic shift of the input data and computing
 * the arithmetic mean of all results shifted back, these artifacts
 * vanish.
 *
 * t=0, soft denoising, wavelet shrinkage
 * t=1, hard denoising, hard thresholding
 */
{
  float *y1;
  double *y2;
  float *y3;
  int i,j,k;
  y1=(float*)malloc(n*sizeof(float));
  y2=(double*)malloc(n*sizeof(double));
  y3=(float*)malloc(n*sizeof(float));
/*
 * Set result vector to 0.0.
 */  
  for (i=0;i<=n-1;i++)
    y2[i]=0.0;
/*
 * any possible cyclic shift
 */     
  for(i=0;i<n;i++)
    {
      if(((i+1)/100)*100==i+1)
        fprintf(stderr,"processing shift %i of %i\n",i+1,n);
      for(j=0;j<n;j++)
        {
          k=j+i;
          if (k>=n)
            k=k-n;
          y1[j]=y[k];
        };
      simpledenoise(y1,n,sf,t);
      for(j=0;j<n;j++)
        {
          k=j+i;
          if (k>=n)
            k=k-n;
          y3[k]=y1[j];
        };
      for(j=0;j<n;j++)
        y2[j]=y2[j]+(double)y3[j];
    };
/*
 * divide by number of added cyclic shift results
 */    
  for (i=0;i<n;i++)
    y[i]=y2[i]/n;
  free(y1);
  free(y2);
  free(y3);
}

void i24out(int iw)
/*
 * writes iw to stdout as 24-bit integer lowest byte first
 */
{ 
  div_t r;
  iw=iw+16777216;
  r=div(iw,256);
  putchar(r.rem);
  r=div(r.quot,256);
  putchar(r.rem);
  r=div(r.quot,256);
  putchar(r.rem);     
}

int i24in()
/*
 * reads an integer from stdin as 24-bit integer lowest byte first
 */
{
  int c0,c1,c2,iw;
  c0=getchar();
  if(c0<0)
    c0=c0+256;
  c1=getchar();
  if(c1<0)
    c1=c1+256;
  c2=getchar();
  if(c2<0)
    c2=c2+256;
  iw=c0+256*(256*c2+c1); 
  if(iw>16777216/2)
    iw=iw-16777216;
  return(iw);
}

void compression(int num,float sf,int n)    
/* 
 * compression
 */    
{ 
  int i;
  float max;
  float *y; /* data points */
  float *w; /* wavelet coefficients */
  y=(float*)malloc(n*sizeof(float));
  w=(float*)malloc(n*sizeof(float));
/*
 * reading of input data
 */
  for(i=0;i<num;i++)
    if(scanf("%e",&y[i]) != 1) abort();  
/*
 * filling of the additional data points
 */
  for(i=num;i<n;i++)
    y[i]=y[num-1];
/*
 * transformation into wavelet space
 */      
  wtrafo(y,w,n);
/*
 * all coefficients representable?
 */
  max=sf*8000000;
  if(max < 0.0)
    max=-max;
  for(i=0;i<n;i++)
    if((w[i] > max)||(w[i] < -max))  
      {
        fprintf(stderr,"Absolute value of scale factor is two low.\n");
        exit(1);
      }
/*
 * write the header
 */
  printf("wzip-1.1\n");
/*
 * output of waveletcoefficients as 24-bit signed integers
 * lowest byte first
 *
 * (int)(w[i]/sf) means that each coefficient with an absolute value
 * lower then the scale factor is set to zero, just like in the
 * denoising procedure.
 */
  for(i=0;i<n-1;i++)  
    i24out((int)(w[i]/sf));
/*
 * The last coefficient which is essentially the arithmetic mean of the
 * coefficients of the input vector is rounded towards the nearest integer
 * after application of the scale factor.
 */
  if(w[n-1]>0)
    i24out((int)(w[n-1]/sf+0.5));
  else  
    i24out((int)(w[n-1]/sf-0.5));
  free(w);
  free(y);   
}

void decompression(int num,float sf,int n)    
/* 
 * decompression
 */    
{ 
  int i;
  char st[256];
  float *y; /* data points */
  float *w; /* wavelet coefficients */
  y=(float*)malloc(n*sizeof(float));
  w=(float*)malloc(n*sizeof(float));
  for(i=0;i<9;i++)
    st[i]=getchar();
  st[8]=0;  
  if(strcmp(st,"wzip-0.1") != 0 && strcmp(st,"wzip-1.0") != 0
    && strcmp(st,"wzip-1.1") != 0)
    {
      fprintf(stderr,"Input is not wzip-0.1 or wzip-1.x format.\n");
      exit(1);
    }
  for(i=0;i<n;i++)
    w[i]=(float)i24in();         
/*
 * transformation from wavelet space
 */
  wbtrafo(w,y,n);
/*
 * multiplication by scale factor and output to stdout
 */
  for(i=0;i<num;i++)
    printf("%e\n",sf*y[i]);
  free(w);
  free(y);
}

void denoising(int num,float sf,int n, int t)    
/* 
 * denoising
 *
 * t=0, soft denoising, wavelet shrinkage
 * t=1, hard denoising, hard thresholding 
 */    
{ 
  int i;
  float *y; /* data points */
  y=(float*)malloc(n*sizeof(float));
/*
 * reading of input data
 */
  for(i=0;i<num;i++)
    if(scanf("%f",&y[i]) != 1) abort();
/*
 * filling of the additional data points
 */
  for(i=num;i<n;i++)
    y[i]=(y[num-1]*(n-i)+y[0]*(i-(num-1)))/(n-num+1);
/*
 * translation invariant denoising with wavelet shrinkage
 */     
  tidenoise(y,n,sf,t);
/*
 * output of denoised data
 */
  for(i=0;i<num;i++)
    printf("%e\n",y[i]);
  free(y);   
}

int main(int nargs, char **args)
{
  int num; /* number of measurements */
  int n;   /* number of coefficients (integral power of two) */
  char *tailptr;
  float sf; /* scale factor (higher scale factor gives stronger compression) */
  if(nargs!=4)
    usage();
/*
 * Fist parameter is the number of measurements from stdin.
 */
  num=strtol(args[2],&tailptr,0);
  if((num < 3)||(tailptr==args[2]))
    usage();  
/*
 * Second parameter is the scale factor.
 */
  sf=strtod(args[3],&tailptr);
  if((sf <= 0.0)||(tailptr==args[3]))
    usage();  
/*
 * number of data points (integral power of two, >=num)
 */
  n=2;
  while(n<num)
    n=2*n;  

  if(!strcmp("-c",args[1]))
    compression(num,sf,n);
  else
    {
      if(!strcmp("-d",args[1]))
        decompression(num,sf,n);
      else
        {  
          if(!strcmp("-dn",args[1]))
            denoising(num,sf,n,0);
          else  
            {  
              if(!strcmp("-hdn",args[1]))
                denoising(num,sf,n,1);
              else  
                usage();
            }
        }
    }    
  return(0);
}
