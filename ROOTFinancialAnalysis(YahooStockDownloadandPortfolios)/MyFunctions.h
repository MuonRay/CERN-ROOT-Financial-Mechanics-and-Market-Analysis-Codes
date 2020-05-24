#ifndef MYFUNCTIONS_H
#define MYFUNCTIONS_H

#include <iostream.h>
#include <iomanip.h>
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TRandom.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TProfile.h"
#include "TF1.h"
#include "TMath.h"
#include "TGraph.h"
#include "TLine.h"
#include "TF1.h"

#include "TStyle.h"
#include "TColor.h"
#include "TCanvas.h"
//#include "THistoMap.h"
#include "TFrame.h"
#include "TLatex.h"
#include <fstream>
#include <vector>



void getAlphaBeta(int Window, TString dir, TString sec1, TString sec2, float &alphaOut, float &betaOut,
		  TGraph &gAlphaOut, TGraph &gBetaOut, TGraph &gClose1Out, TGraph &gClose2Out, TGraph &gSpyOut)
{
  TString fname1 = "Data"; fname1+="/"; fname1+=sec1;  fname1+=".txt";
  TString fname2 = "Data"; fname2+="/"; fname2+=sec2;  fname2+=".txt";
  

  float sigma = -1;
  fstream infile1(fname1,ios::in);
  fstream infile2(fname2,ios::in);
  fstream infileSPY("Data/SPY.txt",ios::in);
  
  // temporary data containers 
  TString Date;
  TString Name;
  float val_Open;
  float val_High;
  float val_Low;	  
  float val_Close;	  
  float val_Volume;
  float val_AdjClose;
  
  // data container for closing prices
  vector<float> vClose1, vClose1_;
  vector<float> vClose2, vClose2_;
  vector<float> vOpen1, vOpen1_;
  vector<float> vOpen2, vOpen2_;
  vector<float> vCloseSPY, vCloseSPY_;
  vector<float> vOpenSPY, vOpenSPY_;
  vector<TString> vDate, vDate_;
  
  int nBars1= 0;
  int nBars2=0;
  int nBars3=0;
  // loop over file and fill graph

  cout << "Opening " << fname1 << endl;
  while (infile1 >> Name >> Date >> val_Open >> val_High >> val_Low >> val_Close >> val_Volume >> val_AdjClose)
    {
      nBars1++;
      vClose1_.push_back(val_AdjClose);     
      vOpen1_.push_back(val_Open);     
      vDate_.push_back(Date);
    }
  cout << "Closed " << fname1 << ", read " << nBars1 << " bars" << endl;
  cout << endl;

  cout << "Opening " << fname2 << endl;
  while (infile2 >> Name >> Date >> val_Open >> val_High >> val_Low >> val_Close >> val_Volume >> val_AdjClose)
    {
      nBars2++;
      vClose2_.push_back(val_AdjClose); 
      vOpen2_.push_back(val_Open); 
      // store closing price to vector
    }
  cout << "Closed " << fname2 << ", read " << nBars2 << " bars" << endl;

  cout << endl;
  cout << "Opening SPY.txt "  << endl;
   while (infileSPY >> Name >> Date >> val_Open >> val_High >> val_Low >> val_Close >> val_Volume >> val_AdjClose)
    {
      nBars3++;
      vCloseSPY_.push_back(val_AdjClose);   
      vOpenSPY_.push_back(val_Open);   
      // store closing price to vector
    }
   cout << "Closed " << "SPY.txt" << ", read " << nBars3 << " bars" << endl;
   cout << endl;

  if (nBars1!=nBars2) 
    {
      cout << "Mismatch in file size" << endl;
      //  return;
    }
  
  // reverse the order
  for (int i=0; i<nBars1; i++)
    {
      vCloseSPY.push_back(vCloseSPY_[nBars1-i-1]);
      vClose1.push_back(vClose1_[nBars1-i-1]);
      vClose2.push_back(vClose2_[nBars1-i-1]);
      vOpenSPY.push_back(vOpenSPY_[nBars1-i-1]);
      vOpen1.push_back(vOpen1_[nBars1-i-1]);
      vOpen2.push_back(vOpen2_[nBars1-i-1]);
      vDate.push_back(vDate_[nBars1-i-1]);
    }
  
  TGraph gratio, gratioDisp, gratioNorm, gClose1, gClose2, gCloseSPY;
  for (int i=0; i<nBars1; i++){
    gCloseSPY.SetPoint(i,i+0.5, vCloseSPY[i]);
    gClose1.SetPoint(i,i+0.5, vClose1[i]);
    gClose2.SetPoint(i,i+0.5, vClose2[i]);
    gratio.SetPoint(i, i+0.5, vClose1[i]/vClose2[i]);
  }
  gClose1Out = gClose1;
  gClose2Out = gClose2;
  gSpyOut = gCloseSPY;

  TGraph gCorrelation, gAlpha, gBeta, gAlphavBeta;
  TGraph gPnL;
  TGraph gLastAlphavBeta;
  
  int nDays = Window;
  int point = 0;
  int point1 = 0;
  TF1 f1("f1","[0] + [1]*x");
  
  TH1F hAlphaDist("hAlphaDist","",100, -5, 5);
  TH1F hBetaDist("hBetaDist","",100, -5, 5);

  TCanvas c;
  gCorrelation.SetMarkerStyle(4);
  int nds = Window;
  int pt = 0;
  for (int i=1; i<nBars1; i++){
    // This is just using the closing prices
    //gCorrelation.SetPoint((point)%(nDays),  100.0*(vCloseSPY[i] - vCloseSPY[i-1])/vCloseSPY[i-1], 
    //			  -100.0*(vClose2[i] - vClose2[i-1])/vClose2[i-1] + 100.0*(vClose1[i] - vClose1[i-1])/vClose1[i-1]);

    
    gCorrelation.SetPoint((point)%(2*nDays),  100.0*(vCloseSPY[i] - vOpenSPY[i])/vOpenSPY[i], 
			  -100.0*(vClose2[i] - vOpen2[i])/vOpen2[i] + 100.0*(vClose1[i] - vOpen1[i])/vOpen1[i]);
    point++;
    gCorrelation.SetPoint((point)%(2*nDays),  100.0*(vOpenSPY[i] - vCloseSPY[i-1])/vCloseSPY[i-1], 
			  -100.0*(vOpen2[i] - vClose2[i-1])/vClose2[i-1] + 100.0*(vOpen1[i] - vClose1[i-1])/vClose1[i-1]);

   
    point++;
    gCorrelation.Draw("AP");
    
    if (i>nDays)
      {
	gCorrelation.Fit("f1");
	float alpha = f1.GetParameter(0);
	float beta = f1.GetParameter(1);
	gAlpha.SetPoint(point1, i, alpha);
	gBeta.SetPoint(point1, i, beta);
	gAlphavBeta.SetPoint(point1%60, alpha, beta);
	if (i>(nBars1-nds-1))
	  {
	    gLastAlphavBeta.SetPoint(pt, alpha,beta);
	    pt++;
	  }

	float PtoL =  fabs(alpha + beta)/fabs(beta - alpha);
	if (PtoL>10) PtoL = 10;
	gPnL.SetPoint(point1, i,PtoL);
	point1++;

	alphaOut = alpha;
	betaOut = beta;
	
	hAlphaDist.Fill(alpha);
	hBetaDist.Fill(beta);
      }
  }
  
  gAlphaOut = gAlpha;
  gBetaOut = gBeta;


  TCanvas d("can","",1000,1200);
  d.Divide(2,4);
  
  d.cd(1);
  d.GetPad(3)->SetGridx(1); 
  d.GetPad(3)->SetGridy(1);
  gClose1.GetYaxis()->SetTitle(sec1+ " (Long)");
  gClose1.Draw("AL");
  d.cd(3);
  d.GetPad(4)->SetGridx(1); 
  d.GetPad(4)->SetGridy(1);
  gClose2.GetYaxis()->SetTitle(sec2+ " (Short)");
  gClose2.Draw("AL");
  d.cd(5);
  d.GetPad(5)->SetGridx(1);
  d.GetPad(5)->SetGridy(1);
  gratio.GetYaxis()->SetTitle("Ratio (" + sec1 + "/" + sec2 + ")" );
  gratio.Draw("ALP");
  d.cd(7);
  d.GetPad(7)->SetGridx(1); 
  d.GetPad(7)->SetGridy(1);
  gCloseSPY.GetYaxis()->SetTitle("SPY");
  gCloseSPY.Draw("AL");
  

  
  d.cd(2);
  d.GetPad(1)->SetGridx(1); 
  d.GetPad(1)->SetGridy(1);
  gAlpha.GetYaxis()->SetTitle("alpha");
  gAlpha.Draw("AL");
  d.cd(4);
  d.GetPad(2)->SetGridx(1); 
  d.GetPad(2)->SetGridy(1);
  gBeta.GetYaxis()->SetTitle("beta");
  gBeta.Draw("AL");
  d.cd(6);
  d.GetPad(6)->SetGridx(1);
  d.GetPad(6)->SetGridy(1);  
  gAlphavBeta.GetXaxis()->SetTitle("alpha");
  gAlphavBeta.GetYaxis()->SetTitle("beta");
  gAlphavBeta.SetMarkerSize(0.5*gAlphavBeta.GetMarkerSize());
  gAlphavBeta.SetMarkerStyle(4);
  gAlphavBeta.Draw("AP");
  gLastAlphavBeta.SetMarkerColor(kRed);
  gLastAlphavBeta.SetLineColor(kRed);
  gLastAlphavBeta.SetMarkerSize(0.5*gLastAlphavBeta.GetMarkerSize());
  gLastAlphavBeta.SetMarkerStyle(4);
  gLastAlphavBeta.Draw("LP");
  d.cd(8);
  d.GetPad(8)->SetGridx(1); 
  d.GetPad(8)->SetGridy(1);
  gCorrelation.SetMarkerStyle(4);
  gCorrelation.SetMarkerSize(0.5*gCorrelation.GetMarkerSize());
  gCorrelation.GetXaxis()->SetTitle("%-change(SPY)");
  gCorrelation.GetYaxis()->SetTitle("%-change(Long-Short)");
  gCorrelation.Draw("AP");

  /*
  d.cd(2);
  hAlphaDist.Draw();
  d.cd(4);
  hBetaDist.Draw();
  d.cd(7);
  */
  //gratio.SetMarkerStyle(4);
  // gratio.Draw("ALP");

  TString canTitle =dir; canTitle+="/"; canTitle+=sec1; canTitle+="_"; canTitle+=sec2; canTitle+=".gif";
  d.Print(canTitle);
  
}



#endif
