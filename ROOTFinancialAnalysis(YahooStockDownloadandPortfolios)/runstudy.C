  // Created by Platowanab (at) gmail (dot) com
  // Date: July 28, 2010
  // This simply reads in data from a long list and short list, then calculates the pairs trade portfolio (Long - Short) alpha and beta. 
  // Using alpha and beta, it then plots the Max and min returns assuming a Max and Min variation of 1% on the SPY as reference points. 
  // The optimal pair may be considered the one with best Min/Max values, though I should probably come up with an actual metric.  




//Inspired by FA some time ago, I began developing some code to eye possible pairs trades. 
//The typical type of pairs trade one reads about is related to finding short-term divergences 
//between two tightly correlated names, then shorting the leader and going long the laggard until 
//the divergence resolves. Sorry, this is not what I'm interested in. I'm interested in going long a
// strong leader (hopefully undergoing accumulation) and shorting a p.o.s. (hopefully undergoing distribution) 
//such that market neutrality isn't necessarily preserved, yet market direction can be somewhat mitigated 
//(or even exploited) by making good pair choices. I like to think that this style of trading suits me since 
//(a) I'm an absolutely terrible day trader and 
//(b) I work as a scientist, and the more I obsess over 1m price charts, the less funding I'll get from the government
// (in fact (b) may explain (a)). So once again, I wrote some ROOT code to help me find "optimal" pairs to trade. 
//Please share ideas to improve the algorithm if you want. I'm happy to incorporate any good ideas. 
//Seeding the algorithm with good long and short candidates is probably 99% of the work when running this,
// and should depend heavily on fundamental analysis, which I'm also terrible at. 

//I'm sure this type of approach (or something like it) is well known to people in the trading business already,
// so I'm not claiming any great innovation here. I'm just trying to develop some simple and usable algorithm 
//to help make some good picks, that even I can understand. The best way to understand something is to try and program it yourself! 
//Please, if I've screwed up something obvious, feel free to tell me. Anyway, here's the (very simple) idea: 
//Compile a list of names to go Long in and a list to go short - which might be based on some mix of TA and FA. 
//Examine all possible combinations of longs and shorts. Look at the %-change in (L - S) vs %-change in the S&P for some Nday window.
// Do a simple linear fit on this scatter plot to find the "alpha" and "beta" for this portfolio. 
//I interpret alpha as the market neutral rate of return for the pair. 
//beta is the change in return versus change in market. Now assume that the market typically changes by at most +-1% (just for simplicity). 
//I'll probably change this to +- 2*RMS of the market change for the past Ndays. 
//Since the linear fit is just a function of alpha and beta, we have %-change(Return) = alpha + beta*(%-change(SPX)), 
//a reasonable Max %-change(Return) = alpha + beta, and Min %-change(Return) = alpha - beta. 
//So for each possible pair, we can look at (alpha + beta) and (alpha - beta) as two metrics for how the trade might do. 
//These metrics are very simple (and naive), but it'll do for my first shot.
// If you've read this far, and you have a great way to quickly find long and short names using TA and FA please let me know.
// Also, if you have a nice metric to optimize on let me know that too. 



//The Goldman Sachs risk system is called SecDB (securities database), and everything at Goldman that matters is run out of it. 
//The GUI itself looks like a settings screen from DOS 3.0, but no one cares about UI cosmetics on the Street. 
//The language itself was called SLANG (securities language) and was a Python/Perl like thing, with OOP and the ORM layer baked in. 
//Database replication was near-instant, and pushing to production was two keystrokes. 
//You pushed, and London and Tokyo saw the change as fast as your neighbor on the desk did 
//(and yes, if you fucked things up, you got 4AM phone calls from some British dude telling you to fix it). 
//Regtests ran nightly, and no one could trade a model without thorough testing (that might sound like standard practice,
// but you have no idea how primitive the development culture is on the Street). 
//Unbeknownst to most of the non-strategists, you could see basically every position and holding across the company, 
//whether you were supposed to or not. The whole thing was so good, I didn’t even know what an ORM really was until
// I started using Rails and had to wrestle with ActiveRecord. 
//The codebase was roughly 15MM lines when I left, and growing.
// I suspect my retinas are still scarred by the weird color blue SecDB was by default. [Source Unknown?]  







  gROOT->Reset();
  
  
  
#include<fstream.h>
#include<iomanip.h>
  
  gROOT->ProcessLine(".L MyFunctions.h+O");

  // Set color palatte options
  gStyle->SetPalette(1);
  const Int_t NRGBs = 5;
  const Int_t NCont = 200;
  Double_t stops[NRGBs] = { 0.00, 0.5, 0.5, 0.84, 1.00 };
//  Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
  Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
  Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
  Double_t blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };
  TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
  
  gStyle->SetPadBorderMode(0);
  gStyle->SetFrameBorderMode(0);
  gStyle->SetNumberContours(NCont);
  
  //////////////////////////////////
  // Number of past days to evaluate 
  int Ndays = 5;
  //////////////////////////////////


  // Make sure your long and short lists are correct here!
  int NLong = 10;
  TString Longs[NLong] = {"ALV","BVF","CHRW","CMI","EXPD","FFIV","INFA","NTY","RVBD","TIE"};

  int NShort = 9;
  TString Shorts[NShort] = {"AU", "BBT", "BK", "CHK", "ESRX", "MDT", "MHS", "NTRS", "RIG"};

    
  TGraph gAlpha[NLong][NShort];
  TGraph gBeta[NLong][NShort];
  TGraph gClose1[NLong][NShort];
  TGraph gClose2[NLong][NShort];
  TGraph gSpy;

  float alpha, beta;
  TH2F hAlpha("hAlpha","", NLong, 0, NLong, NShort, 0, NShort);
  TH2F hAlphaFilter("hAlphaFilter","", NLong, 0, NLong, NShort, 0, NShort);
  TH2F hBeta("hBeta","", NLong, 0, NLong, NShort, 0, NShort);
  TH2F hRiskReward("hRiskReward","", NLong, 0, NLong, NShort, 0, NShort);
  TH2F hProfit("hProfit","", NLong, 0, NLong, 2*NShort, 0, 2*NShort);
  TH2F hLoss("hLoss","", NLong, 0, NLong, NShort, 0, NShort);
  
  float BetaFilter = 0.05;

 
  for (int i=0; i<NLong; i++)
    for (int j=0; j<NShort; j++){
      // if (i!=j)
	{
	  TString dir = "Plots";
	  gSystem->cd(dir);
	  gSystem->mkdir(Longs[i]);
	  dir+="/";
	  dir+=Longs[i];
	  gSystem->cd("../");
	  getAlphaBeta(Ndays,dir,Longs[i],Shorts[j], alpha, beta, gAlpha[i][j], gBeta[i][j], gClose1[i][j], gClose2[i][j], gSpy);
	  
	  hAlpha.Fill(i,j, alpha);
	  hBeta.Fill(i,j,beta);
	  float Profit = alpha + beta;
	  float Loss = alpha - beta;

	  if (Profit > 0 && Loss < 0){
	    float riskReward = fabs(Profit/Loss);
	    if (riskReward > 10) riskReward = 10;
	    
	  }
	  hRiskReward.Fill(i,j, riskReward);

	  hLoss.Fill(i,j,Loss);
	  //  if (Loss > 0 && beta > 0)
	  hProfit.Fill(i,2*j+1,TMath::Max(Loss, Profit));     // max
	  hProfit.Fill(i,2*j,  TMath::Min(Loss, Profit));     // min

	  if (fabs(beta) < BetaFilter)
	    hAlphaFilter.Fill(i,j, alpha);

	  hAlpha.GetXaxis()->SetBinLabel(i+1, Longs[i]);
	  hAlpha.GetYaxis()->SetBinLabel(j+1, Shorts[j]);

	  hBeta.GetXaxis()->SetBinLabel(i+1, Longs[i]);
	  hBeta.GetYaxis()->SetBinLabel(j+1, Shorts[j]);

	  hProfit.GetXaxis()->SetBinLabel(i+1, Longs[i]);
	  hProfit.GetYaxis()->SetBinLabel(2*j+1, Shorts[j] + " Low");
	  hProfit.GetYaxis()->SetBinLabel(2*j+2, Shorts[j] + " High");

	  hLoss.GetXaxis()->SetBinLabel(i+1, Longs[i]);
	  hLoss.GetYaxis()->SetBinLabel(j+1, Shorts[j]);

	  hAlphaFilter.GetXaxis()->SetBinLabel(i+1, Longs[i]);
	  hAlphaFilter.GetYaxis()->SetBinLabel(j+1, Shorts[j]);
	  
	  hRiskReward.GetXaxis()->SetBinLabel(i+1, Longs[i]);
	  hRiskReward.GetYaxis()->SetBinLabel(j+1, Shorts[j]);
	 

	}
    }

  TCanvas c("c","",800,1000);
  c.Divide(1,3);
  c.cd(3);
  c.GetPad(3)->SetGridx(1);
  c.GetPad(3)->SetGridy(1);

  int AlphaFilterMinMax = 1 + TMath::Max(fabs(hAlphaFilter.GetMinimum()), hAlphaFilter.GetMaximum());
  hAlphaFilter.SetStats(0);
  hAlphaFilter.SetMinimum(-AlphaFilterMinMax);
  hAlphaFilter.SetMaximum(AlphaFilterMinMax);
  hAlphaFilter.Draw("COLZTEXT");

  c.cd(3);
  hRiskReward.SetStats(0);
  hRiskReward.Draw("COLZTEXT");

  c.cd(3);
  int ProfitMinMax = 1 + TMath::Max(fabs(hProfit.GetMinimum()), hProfit.GetMaximum());
  hProfit.SetMinimum(-ProfitMinMax);
  hProfit.SetMaximum(ProfitMinMax);
  hProfit.SetStats(0);
  hProfit.GetXaxis()->SetTitle("Long");
  hProfit.GetYaxis()->SetTitle("Short");
  hProfit.SetTitle("High/Low Returns(%) from Pair given #pm1% change in SPY");
  hProfit.Draw("COLZTEXT");

  c.cd(2);
  c.GetPad(2)->SetGridx(1);
  c.GetPad(2)->SetGridy(1);
  int AlphaMinMax = 1 + TMath::Max(fabs(hAlpha.GetMinimum()), hAlpha.GetMaximum());
  hAlpha.SetMinimum(-AlphaMinMax);
  hAlpha.SetMaximum(AlphaMinMax);
  hAlpha.SetStats(0);
  hAlpha.SetTitle("Pair alpha");
  hAlpha.GetXaxis()->SetTitle("Long");
  hAlpha.GetYaxis()->SetTitle("Short");
  hAlpha.Draw("COLZTEXT");
  c.cd(1);
  c.GetPad(1)->SetGridx(1);
  c.GetPad(1)->SetGridy(1);
  int BetaMinMax = 1 + TMath::Max(fabs(hBeta.GetMinimum()), hBeta.GetMaximum());
  hBeta.SetMinimum(-BetaMinMax);
  hBeta.SetMaximum(BetaMinMax);
  hBeta.SetStats(0);
  hBeta.SetTitle("Pair beta");
  hBeta.GetXaxis()->SetTitle("Long");
  hBeta.GetYaxis()->SetTitle("Short");
  hBeta.Draw("COLZTEXT");

  c.Print("AlphaBeta.eps");
  
  TH2F hdum1("hdum1","",gSpy.GetN(), 0,  gSpy.GetN(), 10 , -1,1); 
  TH2F hdum2("hdum2","",gSpy.GetN(), 0,  gSpy.GetN(), 10, -3,3); 


  TCanvas d("d","",500,800);
  d.cd();
  d.Divide(1,3);
  d.cd(1);
  hdum1.SetStats(0);
  hdum1.Draw();
  d.GetPad(1).SetGridx(1);
  d.GetPad(1).SetGridy(1);

  d.cd(2);
  hdum2.SetStats(0);
  hdum2.Draw();
  d.GetPad(2).SetGridx(1);
  d.GetPad(2).SetGridy(1);



  TCanvas e("e","",500,800);
  e.cd();
  e.Divide(1,3);
  e.cd(1);
  hdum1.SetStats(0);
  hdum1.Draw();
  e.GetPad(1).SetGridx(1);
  e.GetPad(1).SetGridy(1);

  e.cd(2);
  hdum2.SetStats(0);
  hdum2.Draw();
  e.GetPad(2).SetGridx(1);
  e.GetPad(2).SetGridy(1);
  int color = 1;
   for (int i=0; i<NLong; i++)
     {
       e.cd(1);
       hdum1.Draw();
       e.cd(2);
       hdum2.Draw();
       for (int j=0; j<NShort; j++)
	 {
	   
	   gAlpha[i][j].SetLineColor(color%30 + 1);
	   gBeta[i][j].SetLineColor(color%30 + 1);
	   
	   d.cd(1);
	   gAlpha[i][j].Draw("L");
	   d.cd(2);
	   gBeta[i][j].Draw("L");
	   color++;
	   
	   e.cd(1);
	   gAlpha[i][j].Draw("L");
	   e.cd(2);
	   gBeta[i][j].Draw("L");	   
	 }
       
       TString dir = "Pairs";
       dir+="/";
       dir+=Longs[i];
       gSystem->cd(dir);
       //  e.Print("AlphaBetaGraphs.eps");
       gSystem->cd("../..");
     }
   d.cd(3);
   d.GetPad(3).SetGridx(1);
   d.GetPad(3).SetGridy(1);   
   gSpy.GetXaxis()->SetRangeUser(0, gSpy.GetN());
   gSpy.Draw("AL");
   //d.Print("AlphaBetaGraphs.eps");
