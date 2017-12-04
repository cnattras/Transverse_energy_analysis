// editing copy of fitBESData5_1.cpp...





// FIXME so far no editing made







#include <iostream>
#include <string>
#include "TKey.h"
#include <fstream>
#include "fitALICE2013Data.h"
using namespace std;
////// FIXME: verify integral values from wolframalpha

// forward declarations for methods in fitBESData.h:
Double_t getdNdptOverptIntegrand(Double_t* rad, Double_t* par);// not used
Double_t getdNdpt(Double_t* pT, Double_t* params);
string concatenateHistoname(string,string,string,string);
Double_t* getIntegralsAndErrorsFromData(TH1D*, Double_t, Double_t);
/////Double_t* getIntegralsAndErrorsFromFit(Double_t* myPt, Double_t* par);
Double_t getdNdpt(Double_t* pT, Double_t* params);
Double_t getdETdEtaIntegrand(Double_t* myPt, Double_t* par);
Double_t getdETdyIntegrand(Double_t* myPt, Double_t* par);
Double_t getdNdEtaIntegrand(Double_t* myPt, Double_t* par);
Double_t getdNdyIntegrand(Double_t* myPt, Double_t* par);
Int_t* getNpartAndErr(Double_t collisionEnergy, string centrality);

// main function:
int fitALICE2013Data_-1(){
	std::ofstream datFile ("ALICE2013Results.dat", std::ofstream::out);
	datFile << "CollEn"<< "\t"	
			<< "particle" << "\t"
			<< "centrality" << "\t"
			<< "mass" << "\t"
			<< "beta" <<"\t"
			<< "betaErr" <<"\t"
			<< "temp" <<"\t"
			<< "tempErr" <<"\t"
			<< "n-v_prof" <<"\t"
			<< "nErr" <<"\t"
			<< "norm" <<"\t"
			<< "normErr" <<"\t"

			<< "dETdEtaTotal" << "\t" // dETdEtaTotal
			<< "dETdEtaTErr" << "\t"
			
			<< "dETdyTotal" << "\t" // dETdyTotal
			<< "dETdyTErr" << "\t"
			
			<< "dNdEtaTotal" << "\t" // dNdEtaTotal
			<< "dNdEtaTErr" << "\t"
			
			<< "dNdyTotal" << "\t" // dNdyTotal
			<< "dNdyTErr" << "\t"
			<< "Npart" << "\t"
			<< "NpartErr" << "\n";
	TFile* myFile = new TFile("ALICE2013Spec.root");
	// object name: cent0_pi_Pb+Pb_2.96T
	TIter next(myFile->GetListOfKeys());
	TKey* mikey;
	TH1D* h;
	TGraphAsymmErrors* tgae;
	TCanvas* c1;
	TClass* class1;
	TF1* funcBGBW;
	TF1* dETdEtaIntegrandFunc;
	TF1* dETdyIntegrandFunc;
	TF1* dNdEtaIntegrandFunc;
	TF1* dNdyIntegrandFunc;
	int breakOutForTesting =0;
	int stop =1; // breakOut after this many iterations (if achieved); default: 140
	while((mikey=(TKey*)next())){// TODO!!! delete mikey at the end of every loop
		class1 = gROOT->GetClass(mikey->GetClassName());
		if(!class1->InheritsFrom("TGraphAsymmErrors")){
			cout << "Object "<< mikey->ClassName() <<" is not TGraphAsymErrors!"<< endl;
			delete class1;
			mikey->DeleteBuffer();
			continue;
		}

		c1 = new TCanvas(); // a la Rademakers
		funcBGBW = new TF1("getdNdpt",getdNdpt,0.00000000000001,10.,6); // actually has 5 parameters
												// 6th parameter, type, multiplied by 0 and added
												// for consistency of cov. matrix needed later		
		dETdEtaIntegrandFunc = new TF1("dETdEtaIntegrand", 
									getdETdEtaIntegrand, 
									0, 10, 6 );// function goes from 0 to 10
										// and has 6 parameters"
										// mass, beta, temp, n, norm, type
		dETdyIntegrandFunc = new TF1("dETdyIntegrand",
								  getdETdyIntegrand,
								  0,10,6);
		dNdEtaIntegrandFunc = new TF1("dETdyIntegrand",
								  getdNdEtaIntegrand,
								  0,10,6); // 5 parameters:m,b,t,n,norm, 6th is type*0
		dNdyIntegrandFunc = new TF1("dETdyIntegrand",
								  getdNdEtaIntegrand,
								  0,10,6);// 5 parameters:m,b,t,n,norm, 6th is type*0
		gPad->SetLogy();
		gStyle->SetOptFit(1111);// display fit parameters; customizable
		gStyle->SetOptDate();// display date (at bottom left)
		gROOT-> SetBatch(kTRUE);// save canvases without displaying them
		c1->Update();
		// read histogram object for current iteration of key:
		tgae = (TGraphAsymmErrors*)mikey->ReadObj();
		string histoName = tgae->GetName();
		Double_t collEn = 0.;// initialize
		//cent8_ka+_Au+Au_7.7 // sample histo name
		if(histoName.substr( histoName.length() - 4 ) == "_7.7") collEn = 7.7;
		else if(histoName.substr( histoName.length() - 4 ) == "11.5") collEn = 11.5;
		else if(histoName.substr( histoName.length() - 4 ) == "19.6") collEn = 19.6;
		else if(histoName.substr( histoName.length() - 4 ) == "u_27") collEn = 27;
		else if(histoName.substr( histoName.length() - 4 ) == "u_39") collEn = 39;
		//get first three characters of particle name from histoName:
		string particleID = histoName.substr(6,3);// starting position in array:6, 3 chars total
		string centrality = histoName.substr(4,1);// starting position in array:4, 1 char total
		
		//------------ Assign mass & type to particle -----------------//
		Double_t mass; // in GeV
		
		// type Double_t instead of Int_t 
		 //to use as argument in TF1 method SetParameters()
		Double_t type;// 0 for mesons, -1 for baryons, 1 for antibaryons
		if		(particleID=="pi-"||particleID=="pi+")
				{mass = 0.13957; type = 0.;}
		else if	(particleID=="ka-"||particleID=="ka+")
				{mass = 0.49368; type = 0.;}
		else if	(particleID=="pro")
				{mass = 0.93827; type = -1.;}
		else if	(particleID=="pba")
				{mass = 0.93827; type = 1.;}
		else if  (particleID == "pis")
				{mass = 0.13957; type = 0.;}
		else {cout << "Check particle: "
				<< particleID<<endl;return 1;}


// TGraphAsymmErrors not binned, so the function getIntegralsAndErrorsFromData won't work
// this function not needed anyway, since taking the functional integral of the data will
// suffice in this case in order to cross check
/*		Double_t* integralDataPtr;
cout << "check6" << histoName<< endl;			

		integralDataPtr = getIntegralsAndErrorsFromData(h,type,mass);
					// ^ method verified!!!

*/		
		//------------- Begin BGBW fit --------------------------//
		if (	histoName == "cent7_ka-_Au+Au_7.7"
			|| 	histoName == "cent7_ka-_Au+Au_11.5"
			||	histoName == "cent7_pi+_Au+Au_7.7"
			||	histoName == "cent8_ka+_Au+Au_7.7"
			||	histoName == "cent4_pi-_Au+Au_19.6"
			||	histoName == "cent5_ka+_Au+Au_27"
			||	histoName == "cent5_ka-_Au+Au_7.7"
			||	histoName == "cent6_pi+_Au+Au_11.5"){
			funcBGBW->SetParameters(mass,0.9,0.03,0.01,10000.,type);
			}			
		else{
			funcBGBW->SetParameters(mass,0.95,0.05,0.1,1000000.,type);
			}
		funcBGBW->SetParNames("mass","beta (c)","temp","n","norm","type");
		funcBGBW->SetParLimits(1,0.5,0.999999999999999999999);//param 1

		funcBGBW->FixParameter(0,mass);// mass in GeV
		funcBGBW->FixParameter(5,type);
		TFitResultPtr r = tgae->GraphFitChisquare("getdNdpt","S","",0.00000000000001,10.);
		Double_t chi2Prob = r->Prob();
		cout << "chi-sq prob: " << chi2Prob << endl;
		tgae->SetMaximum(5*(tgae->GetMaximum()));
		//h-> GetYaxis()->SetRangeUser(0.,maxY);
		//TMatrixDSym cov = r->GetCovarianceMatrix();
		tgae-> GetXaxis()->SetRangeUser(0.,10.);
		TString xlabel = "p_{T}";
		TString ylabel = "#frac{d^{2}N}{dydp_{T}}";
		//tgae-> SetXTitle(xlabel);
		//tgae-> SetYTitle(ylabel);
		Double_t beta 			= funcBGBW->GetParameter(1);
		Double_t temp 			= funcBGBW->GetParameter(2);
		Double_t n	  			= funcBGBW->GetParameter(3);
		Double_t norm 			= funcBGBW->GetParameter(4);
		Double_t betaErr 		= funcBGBW->GetParError(1);
		Double_t tempErr 		= funcBGBW->GetParError(2);
		Double_t nErr 			= funcBGBW->GetParError(3);
		Double_t normErr 		= funcBGBW->GetParError(4);
		//------------- end BGBW fit ----------------------------
		
		
		//-------- Find integrals left and right of data points -------//
		funcBGBW			 	-> SetParameters(mass,beta,temp,n,norm,type);
		dETdEtaIntegrandFunc 	-> SetParameters(mass,beta,temp,n,norm,type);
		dETdEtaIntegrandFunc	-> FixParameter(5,type);
		dETdEtaIntegrandFunc	-> FixParameter(0,mass);
		dETdyIntegrandFunc 		-> SetParameters(mass,beta,temp,n,norm,type);
		dETdyIntegrandFunc		-> FixParameter(5,type);
		dETdyIntegrandFunc		-> FixParameter(0,mass);
		dNdEtaIntegrandFunc 	-> SetParameters(mass,beta,temp,n,norm,type);
		dNdEtaIntegrandFunc		-> FixParameter(0,mass);
		dNdEtaIntegrandFunc		-> FixParameter(5,type);
		dNdyIntegrandFunc 		-> SetParameters(mass,beta,temp,n,norm,type);
		dNdyIntegrandFunc		-> FixParameter(0,mass);
		dNdyIntegrandFunc		-> FixParameter(5,type);
/*
		Int_t totBins 	= h->GetNbinsX();		
		Int_t binx1 	= 0;
		Int_t binx2 	= totBins+1;
		
		Double_t leftCut 	= h->GetXaxis()->GetBinLowEdge(binx1+2); 
		Double_t rightCut 	= h->GetXaxis()->GetBinUpEdge(binx2-1); 
*/		
		Double_t dETdEta 	= dETdEtaIntegrandFunc -> Integral(0.,20.);
		//Double_t dETdEtaRight 	= dETdEtaIntegrandFunc -> Integral(rightCut,10.);
		Double_t dETdy		= dETdyIntegrandFunc -> Integral(0.,20.);
		//Double_t dETdyRight 	= dETdyIntegrandFunc -> Integral(rightCut,10.);
		Double_t dNdEta 	= dNdEtaIntegrandFunc -> Integral(0.,20.);
		//Double_t dNdEtaRight 	= dNdEtaIntegrandFunc -> Integral(rightCut,10.);
		Double_t dNdy 		= dNdyIntegrandFunc -> Integral(0.,20.);
		//Double_t dNdyRight 		= dNdyIntegrandFunc -> Integral(rightCut,10.);
		// Errors:
		Double_t dETdEtaErr	= 
		dETdEtaIntegrandFunc->IntegralError(0.,20.0,
								r->GetParams(),
								r->GetCovarianceMatrix().GetMatrixArray());

		Double_t dETdyErr	= 
		dETdyIntegrandFunc->IntegralError(0.,20.0,
								r->GetParams(),
								r->GetCovarianceMatrix().GetMatrixArray());

		Double_t dNdEtaErr	= 
		dNdEtaIntegrandFunc->IntegralError(0.,20.0,
								r->GetParams(),
								r->GetCovarianceMatrix().GetMatrixArray());

		Double_t dNdyErr	= 
		dNdyIntegrandFunc->IntegralError(0.,20.0,
								r->GetParams(),
								r->GetCovarianceMatrix().GetMatrixArray());

								
		//Double_t dETdEta_d = *(integralDataPtr+0);
		//Double_t dETdEta_d_err = *(integralDataPtr+1);
		Double_t dETdEtaTotal = dETdEta;
		Double_t dETdEtaTErr = dETdEta;
		
		//Double_t dETdy_d = *(integralDataPtr+2);
		//Double_t dETdy_d_err = *(integralDataPtr+3);
		Double_t dETdyTotal = dETdy;
		Double_t dETdyTErr = dETdyErr;
		
		//Double_t dNdEta_d = *(integralDataPtr+4);
		//Double_t dNdEta_d_err = *(integralDataPtr+5);
		Double_t dNdEtaTotal = dNdEta;
		Double_t dNdEtaTErr = dNdEtaErr;
		
		//Double_t dNdy_d = *(integralDataPtr+6);
		//Double_t dNdy_d_err = *(integralDataPtr+7);
		Double_t dNdyTotal = dNdy;
		Double_t dNdyTErr = dNdyErr;
		
		//cout <<"Integral from data for histo "<<breakOutForTesting+1<<": "<<*(integralDataPtr+0)<<endl;// 357.633 for pi minus cent 0
		cout<<"-----------------------------------"<<endl;				
		//------ end Find integrals left and right of data points ----//
		//------ begin - assign Npart and errors from BES paper -----//
		Int_t* NpartAndArrPtr;
		Int_t Npart;
		Int_t NpartErr;
		NpartAndArrPtr = getNpartAndErr(collEn,centrality);
		Npart = *(NpartAndArrPtr+0);
		NpartErr = *(NpartAndArrPtr+1);
		//------ end - assign Npart and errors from BES paper -------// 
		//-- Output results to file-----------------------------
		datFile << collEn << "\t"	
				<< particleID << "\t"
				<< centrality << "\t"
				<< mass << "\t"
				<< beta <<"\t"
				<< betaErr <<"\t"
				<< temp <<"\t"
				<< tempErr <<"\t"
				<< n <<"\t"
				<< nErr <<"\t"
				<< norm <<"\t"
				<< normErr <<"\t"
				
				<< dETdEtaTotal<< "\t" // dETdEtaTotal
				<< dETdEtaTErr << "\t"
				
				<< dETdyTotal<< "\t" // dETdyTotal
				<< dETdyTErr << "\t"
				
				<< dNdEtaTotal << "\t" // dNdEtaTotal
				<< dNdEtaTErr << "\t"
				
				<< dNdyTotal << "\t" // dNdyTotal
				<< dNdyTErr << "\t"
				<< Npart << "\t"
				<< NpartErr << "\n";
		
		//-- end- output results to file------------------------
		c1->Update();
		Double_t chi2BGBW = funcBGBW->GetChisquare();
		Double_t nDFBGBW = funcBGBW->GetNDF();
		Double_t p2 = funcBGBW->GetParameter(2);
		Double_t e2 = funcBGBW->GetParError(2);


	
		//cout << "chi2: " << chi2BGBW << "\nndf: " 
			//<< nDFBGBW<< "\nchi2/ndf: " << chi2BGBW/nDFBGBW <<endl;
	
		/* FIXME */
		string imgPathAndName = "./debugPlots/"+histoName+".png";
				//c1 -> SaveAs("./fittedPlots/trial1.png");
		TImage *png = TImage::Create();// FIXME try to use canvas method instead of png object
		png->FromPad(c1);
		const char* imgPathAndNameConstCharPtr = imgPathAndName.c_str();
		png->WriteImage(imgPathAndNameConstCharPtr);
		/**/
		//cout << "Draw class here: \n";
		//h-> DrawClass();
		///////h->Delete();// works
		///////////FIXME c1->Clear();// 
		/// sometimes when you delete objects, they stay in the program stack
		//FIXME delete png;
		mikey->DeleteBuffer();// works!
		breakOutForTesting++;
		if(breakOutForTesting>=stop) break;
		
		gSystem->ProcessEvents();
		delete h;
		delete funcBGBW;
		delete dETdEtaIntegrandFunc;
		delete dETdyIntegrandFunc;
		delete c1;	// Rademakers
		//delete mikey; // FIXME 9 segmentation violation
		//delete class1; // segmentation violation
	}
	//}// end of while loop to iterate through every key
	/////////////delete c1;
	//delete mikey;
	//delete h;
	//delete class1;
	//delete funcBGBW;
	//delete dETdEtaIntegrandFunc;
	//delete dETdyIntegrandFunc;
	////////////////gObjectTable->Print();
	delete myFile;
	datFile.close();
return 0;
}





