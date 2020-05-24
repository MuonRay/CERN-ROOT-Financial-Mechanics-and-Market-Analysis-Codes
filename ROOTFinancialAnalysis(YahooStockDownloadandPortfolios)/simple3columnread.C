

{
// Created by Platowanab (at) gmail (dot) com
// Date: May 17, 2010
// This ROOT script reads in data file "SPX.txt" and creates a simple graph

// load the input file
fstream infile("SPX.txt",ios::in);

// temporary data containers 
TString Date;
float val_Close;
float val_Volume;

TGraph myGraph; // define the graph to fill
int nBars=0;
// loop over file and fill graph
while (infile >> Date >> val_Close >> val_Volume)
{
nBars++;
myGraph.SetPoint(nBars-1, nBars-1, val_Close); // use a graph
}

TCanvas myCanvas; // make a canvas to draw the graph
// set some axis labels
myGraph.GetXaxis()->SetTitle("Hour");
myGraph.GetYaxis()->SetTitle("Close(SPX)");
myGraph.GetYaxis()->SetTitleOffset(1.3);

myGraph.Draw("AL"); // draw the graph with axes ("A") and use a line ("L"). 


}
