{
// Created by Platowanab (at) gmail (dot) com
// Date: May 17, 2010
// This ROOT script reads in data file "AAPL.txt" and creates 2-D
// graph of showing the High to Low price range for each day, where the
// color indicates the day's volume

// This is adjust the color palette
gStyle->SetPalette(1);
const Int_t NRGBs = 5;
const Int_t NCont = 255;

Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
Double_t red[NRGBs] = { 0.00, 0.00, 0.87, 1.00, 0.51 };
Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
Double_t blue[NRGBs] = { 0.51, 1.00, 0.12, 0.00, 0.00 };
TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);

gStyle->SetPadBorderMode(0);
gStyle->SetFrameBorderMode(0);
gStyle->SetNumberContours(NCont);

// load the input file
fstream infile("AAPL.txt",ios::in);

// temporary data containers 
TString Date;
float val_Open;
float val_High;
float val_Low; 
float val_Close; 
float val_Volume;
float val_AdjClose;

// define the data histogram
TH2F myHisto("myHisto","",600,0,600, 800,0,400);
// a dummy histogram 
TH1F dummy("dummy","",myHisto.GetNbinsY(), myHisto.GetYaxis().GetXmin(), myHisto.GetYaxis().GetXmax());

int nBars=0;
// loop over file and fill graph
while (infile >> Date >> val_Open >> val_High >> val_Low >> val_Close >> val_Volume >> val_AdjClose)
{
val_Volume = val_Volume/1.0E6; // convert volume to millions
nBars++;

int binHigh = dummy.FindBin( val_High);
int binLow = dummy.FindBin(val_Low);

for (int j=binLow; j<=binHigh; j++) 
myHisto.SetBinContent(nBars, j, val_Volume);
if (nBars%15 == 0)
myHisto.GetXaxis()->SetBinLabel(nBars, Date); 

}

TCanvas myCanvas; // make a canvas to draw the graph
// set grid lines
myCanvas.SetGridy(1);
myCanvas.SetGridx(1);
// set some axis labels
myHisto.GetZaxis()->SetTitle("Daily Volume (millions)");
myHisto.GetXaxis()->SetTitle("Day");
myHisto.GetXaxis()->SetTitleOffset(2);
myHisto.GetYaxis()->SetTitle("Price");
myHisto.GetYaxis()->SetTitleOffset(1.3);
// turn of statistics display
myHisto.SetStats(0);

myHisto.Draw("COLZ"); // draw the graph the histogram

} 
