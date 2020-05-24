{


  // Created by Platowanab@gmail.com
  // Date: May 17, 2010
  // This ROOT script reads in data file "SPX.txt" and creates a simiple
  // animated gif file showing the Price vs Volume movement of the S&P over the last 3 years.
  // A 50-day moving average line is also drawn over the scatter plot.
  // To run the script, open root, at prompt issue command ".x run_SPX_study.C"  
  // To create your own animated gif uncomment command at bottom "c.Print("plot.gif+")";




  gROOT->Reset();
#include <fstream>
#include <iomanip>
  
  // This is adjust the color palette
  gStyle->SetPalette(1);
  const Int_t NRGBs = 5;
  const Int_t NCont = 255;
  
  Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
  Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
  Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
  Double_t blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };
  TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
  
  gStyle->SetPadBorderMode(0);
  gStyle->SetFrameBorderMode(0);
  gStyle->SetNumberContours(NCont);
  
  // finished with color palette settings


  // load the input file
  fstream infile("SPX.txt",ios::in);
  
  // temporary data containers 
  TString Date;
  float val_Close;
  float val_Volume;


  // data containers
  vector<TString> vDate;
  vector<float> vClose;
  vector<float> vVolume;




  int nDays = 0;
  // read file
  while (infile >> Date >> val_Close >> val_Volume)
    {
      nDays++;
      cout << "Storing data for day : " << nDays << endl;
      cout << "Date:      " << Date << endl;
      cout << "Close:     " << val_Close << endl;
      cout << "Volume:    " << val_Volume << endl;
      cout << endl;


      


      // push data into vector containers
      vDate.push_back(Date);
      vClose.push_back(val_Close);
      val_Volume = val_Volume*1E-9; // make Volume in units of billion 
      vVolume.push_back(val_Volume);
    }


  // make histogram
  TH2F hdata("hdata","",5E1,1E9*1E-9,1E10*1E-9,0.05*(1600-600),600,1600);
  
  // turn off histo statistics view
  hdata.SetStats(0);
  // label axes
  hdata.GetXaxis()->SetTitle("Volume (Billion)");
  hdata.GetYaxis()->SetTitle("SPX");
  hdata.GetYaxis()->SetTitleOffset(1.3);
  hdata.GetZaxis()->SetTitle("Number of Days");


  // make canvas
  float scale = 1.0;
  TCanvas c("c","",100,100,700*scale,600*scale);
  // turn on grid lines
  c.SetGridy(1);
  c.SetGridx(1);


  // make graph
  TGraph gMA;
  int pt=0;


  // make little circle marker
  TMarker marker(0,0,4);
  
  // number of days in SMA
  int nMA  = 50;
  // initialize SMA data
  float VolMA = 0;
  float PriceMA =0;
  
  // loop over data
  for (int i=0; i<nDays;i++)
    {
      // make histogram title
      TString title = ""; title+=vDate[i];
      hdata.SetTitle(title);
     
      // fill close v volume data to histogram
      hdata.Fill(vVolume[i],vClose[i]);
      
      // remove last point from SMA
      if (i>(nMA-1))
        {
          VolMA = VolMA - ((1/(float)nMA)*vVolume[i-nMA]);
          PriceMA = PriceMA - ((1/(float)nMA)*vClose[i-nMA]);
        }
      
      // add new point to SMA
      VolMA += ((1/(float)nMA)*vVolume[i]);
      PriceMA += ((1/(float)nMA)*vClose[i]);


      // fill SMA data into graph
      if (i>(nMA-2))
        {
          gMA.SetPoint(pt, VolMA, PriceMA);
          pt++;
          marker.SetX(VolMA);
          marker.SetY(PriceMA);
        }
      
      // go to canvas
      c.cd();
      // draw histo
      hdata.Draw("COLZ");
      // draw SMA graph line
      gMA.SetLineWidth(2);
      gMA.Draw("L");
      marker.Draw();
      // update canvas
      c.Update();
      // uncomment to make animated gif
        //    c.Print("plot.gif+");

