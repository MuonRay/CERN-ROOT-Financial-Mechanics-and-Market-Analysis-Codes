
void MonteCarloPrediction()
{



// Created by Platowanab (at) gmail (dot) com
// Date: May 22, 2010
// This ROOT script reads in data file "AAPL-small.txt" which contains 32 days of closing prices. 
// The script then takes all 32 days, produces a log-normal histogram which is fit with a Gaussian
// to get the volitility (sigma) and drift (mu) assuming Geometric Brownian Motion (GBM). 
// Once these two parameters are obtained from the data, a simple Monte Carlo model is run
// to produce 5%, 50%, and 95% CL limits on future price action. On top of the CL contours, 
// 10 world lines of possible future price histories are also drawn for fun.
// If you make money using this, tell me how so I can make some too! 

//gROOT->Reset();





// Set color palatte options
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
fstream infile("AAPL-small.txt",ios::in);

// temporary data containers 
TString Date;
float val_Open;
float val_High;
float val_Low; 
float val_Close; 
float val_Volume;
float val_AdjClose;

// data container for closing prices
vector vClose;

// dummy histogram for plot axes formatting
TH2F hdummy("hdummy","",50,0,50,500,0,500);
hdummy.GetYaxis()->SetTitle("Close");
hdummy.GetYaxis()->SetTitleOffset(1.3);
hdummy.GetXaxis()->SetTitle("Date");
hdummy.GetXaxis()->SetTitleOffset(1.5);
hdummy.GetZaxis()->SetTitle("Number of Monte Carlo Trials");

// define graph 
TGraph gClose;
// define log normal histogram for GBM model fit
TH1F hLogdPrice("hLogdPrice","",50,-0.5,0.5);
int nBars=0;
// loop over file and fill graph
while (infile >> Date >> val_Open >> val_High >> val_Low >> val_Close >> val_Volume >> val_AdjClose)
{
// fill graph with closing prices
gClose.SetPoint(nBars, nBars, val_Close);
nBars++;
// store closing price to vector
vClose.push_back(val_Close); 
// fill log normal histogram
if (nBars>1)
hLogdPrice.Fill( TMath::Log(vClose[nBars-1]) - TMath::Log(vClose[nBars-2]) );

if (nBars%3==0)
hdummy.GetXaxis()->SetBinLabel(nBars, Date);
}

//---------Do the GBM fit-----------------------//
// Draw log normal histogram
hLogdPrice.SetFillColor(15);
hLogdPrice.GetXaxis()->SetTitle("Log(Close_{i}/Close_{i-1})");
hLogdPrice.GetYaxis()->SetTitle("Number of Entries");
hLogdPrice.Draw();
// define fit function for log normal fit
cout << endl; cout << endl;
cout << "***********Geometric Brownian Motion fit results************" << endl;
TF1 f("f","gaus");
// do fit
hLogdPrice.Fit("f");
// get fit mean
float par1 = f.GetParameter(1);
// get fit sigma
float par2 = f.GetParameter(2);
// define weiner process sigma
float sigma = par2/sqrt(1);
// define weiner process drift rate 
float mu = (par1/1)+pow(sigma,2)/2;
//----------------------------------------------//

//------Do monte carlo trials-------------------//
// number of days in future to model
float ndays = 10;
// last data point from price history
float closePrice = gClose.GetY()[nBars-1];

// define world lines for fun
TGraph gMCtrials[100];
// how many monte carlo trials
int ntrials = 1E5; 
// random number generator
TRandom3 ran;
// define histo for probabilities
TH2F hMCtrials("hMCtrials","",nBars+ndays,-0.5,nBars+ndays-0.5,10000,0,500);

// 5%, 50%, and 95% CL contour lines
TGraph g50per;
TGraph g5per;
TGraph g95per;
// set first point to the last available close in dataset
g50per.SetPoint(0, nBars-1, closePrice);
g5per.SetPoint(0, nBars-1, closePrice);
g95per.SetPoint(0, nBars-1, closePrice);

float price_ = closePrice;
for (int i=0;i<10;i++) gMCtrials[i].SetPoint(0, nBars-1, closePrice);

// Run the GBM model and collect results in 2-D histogram. Also store 10 example world lines for fun.
for (int j = 0; j< ntrials; j++)
{
price_ = closePrice;
for (int i = 1; i<=ndays; i++)
{
float epsilon = ran.Gaus();
price_ = price_*(1 + mu*i + epsilon*sigma*sqrt(i));
hMCtrials.Fill(nBars+i-1, price_);
if (j<10) gMCtrials[j].SetPoint(i, nBars+i-1, price_);
}

}

// ----- Here is where the 5%, 50%, and 95% contours are found by numerical integration of the monte carlo trials
int point = 1;
for (int i=nBars; i<=hMCtrials.GetNbinsX(); i++)
{
TString name = "tmp"; name+=i;
TH1F *tmp_ = (TH1F*)hMCtrials->ProjectionY(name,i,i);
float integral = tmp_->Integral();
int bin5per = -1;
int bin50per = -1;
int bin95per = -1; 
if (integral>0)
{
for (int j=1; j<=hMCtrials.GetNbinsY(); j++)
{
float int_ = tmp_->Integral(1,j);
float frac = int_/integral;
if (frac < 0.05) bin5per = j; 
if (frac < 0.50) bin50per = j;

float int2_ = tmp_->Integral(hMCtrials.GetNbinsY()-(j-1), hMCtrials.GetNbinsY());
float frac2 = int2_/integral;
if (frac2 < 0.05) bin95per = hMCtrials.GetNbinsY()-(j-1);
}
float limit5per = tmp_->GetBinCenter(bin5per);
float limit95per = tmp_->GetBinCenter(bin95per);
float limit50per = tmp_->GetBinCenter(bin50per);
g95per.SetPoint(point, i-1, limit95per);
g50per.SetPoint(point, i-1, limit50per);
g5per.SetPoint(point, i-1, limit5per);
point++;
}
}

// Draw the outputs
TCanvas c;
c.SetGridx(1);
c.SetGridy(1);

hdummy.SetStats(0);
hdummy.Draw();
hMCtrials.SetStats(0);
hMCtrials.GetZaxis()->SetTitle("Number of Monte Carlo trials");
hMCtrials.GetZaxis()->SetTitleOffset(1.5);
hMCtrials.Draw("COLZSAME");
gClose.SetLineWidth(2);
gClose.Draw("L");
for (int i=0;i<10;i++) {gMCtrials[i].SetLineStyle(2); gMCtrials[i].SetLineWidth(1.2);}
for (int i=0;i<10;i++) gMCtrials[i].Draw("L");
g5per.SetLineWidth(2); 
g50per.SetLineWidth(2);
g95per.SetLineWidth(2);

g50per.SetMarkerStyle(5);
g5per.SetMarkerStyle(5);
g95per.SetMarkerStyle(5);

g50per.Draw("L");
g5per.Draw("L");
g95per.Draw("L");
} 
}
