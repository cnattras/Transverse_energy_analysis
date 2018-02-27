// evolved from interpolateLaCent.cpp, which did not contain errors
// constructor example: 
//TGraphErrors *tg = new TGraphErrors(xNum, &xVec[0], &yVec[0], NULL, &yVecErrStat[0])
#include "Riostream.h"
#include "fitBESData5.h" // for the function getNpartAndErr
//#include <cstdio>
//#include <vector>
#include <string>
#include <fstream>
//#include "TString.h"
//#include "TMath.h"
//#include "TH1D.h"
#include <iostream> // to use cout for debugging
using namespace std;

std::string doubToString(Double_t);
std::string centIndToPercent(int centInd);
void formatGraph(TGraphErrors* g, Double_t collEn_Or_NpartArr[], int en_Or_centInd);
Double_t* interpolateNpartGraph(TGraphErrors* tg, int en);
Int_t* getNpartAndErr(Double_t collisionEnergy, int centrality);
void printInterpolatedGraph(TGraphErrors* tg, int en, Double_t* x_interp, Double_t* y_interp, Double_t* x_interp_err, Double_t* y_interp_err);

Double_t collEnArr[5] = {7.7,11.5,19.6,27,39}; // To use in TGraphErrors

// main function:
int interpLaCentTGE(){
	std::ofstream datfile ("lambdasInterpWithErr.dat", std::ofstream::out);
	cout << "CollEn" << "\t"
		<< "dETdEta1" << "\n";
	datfile << "CollEn"<< "\t"	
			//<< "particle" << "\t"
			//<< "centrality" << "\t"
			//<< "npart" << "\t"
			//<< "npart_err" << "\t"
			<< "dETdEta1" << "\t"
			<< "dETdEta2" << "\t"
			<< "dETdEta3" << "\t"
			<< "dETdEta4" << "\t"
			<< "dETdEta1Err" << "\t"
			<< "dETdEta2Err" << "\t"
			<< "dETdEta3Err" << "\t"
			<< "dETdEta4Err" << "\n";
			//<< "dETdEtaByNpartOver2_err" << "\n"; FIXME
	ifstream in;
	string skipContent;// read and skip content that has no use
	const int cents = 7; // 7 different centralities for la only
	//const int TRUECENTS = 9;
	const int collEns = 5; // 5 different collision energies
	const int parts = 2; // 2 different particles: la and ala
	const int funcsOfCollEn = 1; // there is one quantity that
							// is to be plotted as a function of
							// collision energy:
							// (dET/dEta)/(N_part),
	// array for energy-dependent plots:
	// Don't think this is needed:
	// Double_t enPlotArr[cents][collEns][funcsOfCollEn];
	in.open(Form("./fitResults_la.dat"));
	if (!in.good()){
		cout << "Problem opening file!" << endl;
		return 1;
	}

	for(int i=0;i<47;i++){ // loop through 47 column names
		in >> skipContent;
		// (tested) print first and last header names to check:
		//if (i==0) cout << "Col. 1: " << skipContent << endl;
		//else if (i==45) cout << "Col. 45: " << skipContent << endl;
	}
	Double_t collEn;
	Double_t cent;// centrality value obtained from data file
	string part; // particle name
	Double_t dETdEtaSum[cents][collEns] = {0.};// initialize element [0][0] to 0, 
											//and initialize all other elements as if 
											// they had static storage duration, hence
											// set them to zero as well
	// FIXME: faulty assignment of values to centIndex 6 & 7. Fix after interpolation										
	Double_t dETdEtaSum_errSq[cents][collEns] = {0.}; // errors added in quadrature
	Double_t dETdEtaSum_err[cents][collEns] = {0.}; // sq root of the above
	Double_t dNchdEtaSum[cents][collEns] = {0.};
	Double_t dNchdEtaSum_errSq[cents][collEns] = {0.}; // errors added in quadrature
	Double_t dNchdEtaSum_err[cents][collEns] = {0.}; // sq root of the above
	Double_t Npart[cents][collEns] = {0.};// Npart function of cent and en only
	Double_t Npart_err[cents][collEns] = {0.};
	Double_t dETdEtaOverNpartBy2Sum[cents][collEns] = {0.};
	Double_t dETdEtaOverNpartBy2Sum_err[cents][collEns] = {0.}; // use formula involving...
					//partial derivatives to evaluate uncertainty
	Double_t dETdEtaOverdNchdEtaSum[cents][collEns] = {0.};
	Double_t dETdEtaOverdNchdEtaSum_err[cents][collEns] = {0.}; // 
	// Add elements below (for all 6 particles)
		// in order to get the above elements
	Double_t dETdEta[cents][collEns][parts] = {0.};
	Double_t dETdEtaErr[cents][collEns][parts] = {0.};
	Double_t dNchdEta[cents][collEns][parts] = {0.};
	Double_t dNchdEtaErr[cents][collEns][parts] = {0.};
	Double_t centArr[cents] = {0.}; // need to pass to formatGraph function
	//Double_t dETdEtaOverNpartBy2[cents][collEns][parts];
	// enPlotArr element indexed as: 
	// enPlotArr[centIndex][enIndex][funcIndex]
	int centIndex;
	int enIndex;
	int partIndex;
	int funcIndex;
	
	for(int j=0; j<70; j++){// for 70 rows below header in the data file
		//for(int i=0; i<46; i++){
		in >> collEn; cout << collEn << "\t";
		in >> part; cout << part << "\t";
		in >> cent; cout << cent << "\t";
		for(int k=0; k<15;k++){// skip content in next 15 columns
		in >> skipContent;
		}
		
		// now adding things up
		
		// for each centrality there are 5 different energies
		// for each energy corresponding to a particular centrality,
			// there are 6 different particles, and hence
			// 6 different values of (dET/dEta)/(N_part/2)
		
		// for every collision energy, there are 9 Nparts 
			//(corresponding to 9 centralities)
		// add dETdEtaOverNpartby2 and dETdEtaOverdNchdEta values
			// corresponding to each Npart (cent) for a given collEn
			
		if(cent == 0.) centIndex = 0;
		else if(cent == 1.) centIndex = 1;
		else if(cent == 2.) centIndex = 2;
		else if(cent == 3.) centIndex = 3;
		else if(cent == 4.) centIndex = 4;
		else if(cent == 5.) centIndex = 5;
		else if(cent == 6.) centIndex = 6;
		else if(cent == 7.) centIndex = 7;
		else if(cent == 8.) centIndex = 8;
		else{cout << "Check centrality value!" << endl; return 2;}
		// energy set: {7.7,11.5,19.6,27,39}
		if (collEn == 7.7) enIndex = 0;
		else if (collEn == 11.5) enIndex = 1;
		else if (collEn == 19.6) enIndex = 2;
		else if (collEn == 27) enIndex = 3;
		else if (collEn == 39) enIndex = 4;
		// particle set: {ka-,ka+,pi-,pi+,pro,pba}
		/*
		if 		(part == "pi-") partIndex = 0;
		else if (part == "pi+") partIndex = 1;
		else if (part == "ka-") partIndex = 2;
		else if (part == "ka+") partIndex = 3;
		else if (part == "pro") partIndex = 4;
		else if (part == "pba") partIndex = 5;
		*/
		if 		(part == "ala") partIndex = 6;
		else if (part == "la_") partIndex = 7;
		in >> dETdEta[centIndex][enIndex][partIndex];
		cout << dETdEta[centIndex][enIndex][partIndex] << "\t";
		in >> dETdEtaErr[centIndex][enIndex][partIndex];
		for(int l=0; l<14; l++) in >> skipContent;
		in >> dNchdEta[centIndex][enIndex][partIndex];
		cout << dNchdEta[centIndex][enIndex][partIndex] << "\t";
		in >> dNchdEtaErr[centIndex][enIndex][partIndex];
		for(int l=0; l<8; l++) in >> skipContent;
		in >> Npart[centIndex][enIndex];
		cout << Npart[centIndex][enIndex] << "\t";
		in >> Npart_err[centIndex][enIndex];
		cout << Npart_err[centIndex][enIndex] << "\n";
		in >> skipContent; // last column only contains the fit status

		// TODO: what about lambdas while adding ET?
		// ET = 3ET_pi + 4ET_k + 4ET_p + 2ET_lam (last term is valid if there's no antilambda)
		// however lambda spectra available for centralities different from the rest
		// At high energies, number of negative and positive charges in the collision
			// product are roughly the same
			// which is not true at low energies since the colliding nuclei have +ve nucleons
		// TODO: if the error bars don't look reasonable, cout results to check the errors
		if(partIndex==6 || partIndex==7){
			// lambdas are not charged particles:
			dETdEtaSum[centIndex][enIndex] 			+= dETdEta[centIndex][enIndex][partIndex];
			dETdEtaSum_errSq[centIndex][enIndex] 	+= (dETdEtaErr[centIndex][enIndex][partIndex])*
												 	   (dETdEtaErr[centIndex][enIndex][partIndex]);
			dETdEtaSum_err[centIndex][enIndex]		=  TMath::Sqrt(dETdEtaSum_errSq[centIndex][enIndex]);
			 
		}
	} //end of for loop with index j - captured all 70 rows from data file
	in.close();// all information from file extracted, so it should be closed
	// print dETdEtaSum for 7 centralities and 5 collision energies
		// and assign values to dETdEtaOverNpartBy2Sum[i][j]
	// for quantity q = a/b (such as dETdEtaOverNpartBy2Sum)
	// q_err = sqrt((a_err/b)^2 + (-a*b_err/(b^2))^2)
	for(int i=0; i<cents; i++){
		for(int j=0; j<collEns; j++){
			cout << "dETdEtaSum["<<i<<"]["<<j<<"] 	= "<<dETdEtaSum[i][j]<<endl;
			cout << "dNchdEtaSum["<<i<<"]["<<j<<"] 	= "<<dNchdEtaSum[i][j]<<endl;
			dETdEtaOverNpartBy2Sum[i][j] 		= dETdEtaSum[i][j]/Npart[i][j];
			dETdEtaOverNpartBy2Sum_err[i][j]	= TMath::Sqrt(
													(dETdEtaSum_err[i][j]/Npart[i][j])*
													(dETdEtaSum_err[i][j]/Npart[i][j])
															 +
													(-dETdEtaSum[i][j]*Npart_err[i][j]/
														(Npart[i][j]*Npart[i][j]))*
													(-dETdEtaSum[i][j]*Npart_err[i][j]/
														(Npart[i][j]*Npart[i][j]))
															 );
			dETdEtaOverdNchdEtaSum[i][j] 		= dETdEtaSum[i][j]/dNchdEtaSum[i][j];
			dETdEtaOverdNchdEtaSum_err[i][j]	= TMath::Sqrt(
													(dETdEtaSum_err[i][j]/dNchdEtaSum[i][j])*
													(dETdEtaSum_err[i][j]/dNchdEtaSum[i][j])
															 +
													(-dETdEtaSum[i][j]*dNchdEtaSum_err[i][j]/
														(dNchdEtaSum[i][j]*dNchdEtaSum[i][j]))*
													(-dETdEtaSum[i][j]*dNchdEtaSum_err[i][j]/
														(dNchdEtaSum[i][j]*dNchdEtaSum[i][j]))
															 );
		}
	}
	

	/*
	for(int i=0; i<cents; i++){ // i is cent index
		for(int j=0; j<collEns;j++)
		Npart[i][j] = 
	}
	*/

	/// ------ begin - plot all snn graphs at once -------------------------------//
	for(int centInd=0; centInd<cents; centInd++){ // loop through all centralities
		centArr[centInd] = centInd*1.0; // to avoid double-int conflict
		Double_t dETdEtaOverNpartBy2SumCentByCent[collEns];// one graph per centInd
		Double_t dETdEtaOverNpartBy2SumCentByCent_err[collEns];
		Double_t dETdEtaOverdNchdEtaSumCentByCent[collEns];
		for(int i=0; i<collEns; i++){ // loop through all collEns for current centrality
		// because TGraphErrors does not take a 2d array as argument
			dETdEtaOverNpartBy2SumCentByCent[i] = dETdEtaOverNpartBy2Sum[centInd][i];
			dETdEtaOverNpartBy2SumCentByCent_err[i] = dETdEtaOverNpartBy2Sum_err[centInd][i];
			dETdEtaOverdNchdEtaSumCentByCent[i] = dETdEtaOverdNchdEtaSum[centInd][i];
		}
		TGraph* g1;
		g1 = new TGraph(5, collEnArr, dETdEtaOverNpartBy2SumCentByCent);
		g1 -> SetName("dETdEtaOverNpartBy2Sum_vs_en"); // unique identifier to be used in function:
		cout << "Now formatting graph: " << g1->GetName() << endl;
		// TODO formatGraph(g1, centArr, centInd);
		delete g1;	
	
		// TODO make a function to take care of different types of graph instead of creating new objects here
		TGraph* g2;
		g2 = new TGraph(5, collEnArr, dETdEtaOverdNchdEtaSumCentByCent);
		g2 -> SetName("dETdEtaOverdNchdEtaSum_vs_en"); // unique identifier to be used in function:
		cout << "Now formatting graph: " << g2->GetName() << endl;
		// TODO formatGraph(g2, centArr, centInd);
		delete g2;
		
	} // end of for loop with index centInd
	/// ------ end - plot all snn graphs at once -------------------------------//
	
	Double_t NpartArrEnByEn[cents] = {0.}; // for each collEn, there is a unique Npart Array
	Double_t NpartArrEnByEn_err[cents] = {0.};
	/// ------ begin - plot all Npart graphs at once -------------------------------//
	for(int enInd=0; enInd<collEns; enInd++){ // loop through all coll Ens
	
		Double_t dETdEtaOverNpartBy2SumEnByEn[cents];// one graph per collEn;
		Double_t dETdEtaOverNpartBy2SumEnByEn_err[cents];
		// FIXME currently just npart instead of npartBy2
		Double_t dETdEtaOverdNchdEtaSumEnByEn[cents];	// cents = num of Nparts
		for(int i=0; i<cents; i++){ // loop through all cents/Nparts for current collEn
		// because TGraphErrors does not take a 2d array as argument
			dETdEtaOverNpartBy2SumEnByEn[i] = dETdEtaOverNpartBy2Sum[i][enInd];
			dETdEtaOverNpartBy2SumEnByEn_err[i] = dETdEtaOverNpartBy2Sum_err[i][enInd];
			dETdEtaOverdNchdEtaSumEnByEn[i] = dETdEtaOverdNchdEtaSum[i][enInd];
			NpartArrEnByEn[i]				= Npart[i][enInd];
			NpartArrEnByEn_err[i]			= Npart_err[i][enInd];
		}

		TGraphErrors* g1; // okay to use identifier g1 in local scope as was the case in previous use
		//TGraphErrors *tg = new TGraphErrors(xNum, &xVec[0], &yVec[0], NULL, &yVecErrStat[0])
		g1 = new TGraphErrors(cents/*=7*/, NpartArrEnByEn, dETdEtaOverNpartBy2SumEnByEn, 
								NpartArrEnByEn_err, dETdEtaOverNpartBy2SumEnByEn_err);
		cout << "npartArr: "; 
		for (int i = 0; i<cents; i++){
			cout << *(NpartArrEnByEn+i) << ", ";
		}
		cout << endl;
		cout << "npartArr_err: "; 
		for (int i = 0; i<cents; i++){
			cout << *(NpartArrEnByEn_err+i) << ", ";
		}
		cout << endl;
		cout << "dETdEtaOverNpartArr: "; 
		for (int i = 0; i<cents; i++){
			cout << *(dETdEtaOverNpartBy2SumEnByEn+i) << ", ";
		}
		cout << endl;
		cout << "dETdEtaOverNpart_errArr: "; 
		for (int i = 0; i<cents; i++){
			cout << *(dETdEtaOverNpartBy2SumEnByEn_err+i) << ", ";
		}
		cout << endl;
		Double_t* interpPtr = interpolateNpartGraph(g1, enInd);///////////////// HERE TODO TODO TODO TODO TODO TODO TODO TODO TODO

		datfile << collEnArr[enInd]<< "\t"	
				<< *(interpPtr+0) << "\t"
				<< *(interpPtr+1) << "\t"
				<< *(interpPtr+2) << "\t"
				<< *(interpPtr+3) << "\t"
				<< *(interpPtr+4) << "\t"
				<< *(interpPtr+5) << "\t"
				<< *(interpPtr+6) << "\t"
				<< *(interpPtr+7) << "\n";
				//<< "dETdEtaByNpartOver2_err" << "\n"; // FIXME
		g1 -> SetName("dETdEtaOverNpartBy2Sum_vs_Npart"); // unique identifier for graph
														// see use in function formatGraph
		formatGraph(g1, collEnArr, enInd);
		
		delete g1;
		
		TGraph* g2;
		g2 = new TGraph(cents, NpartArrEnByEn, dETdEtaOverdNchdEtaSumEnByEn);
		g2 -> SetName("dETdEtaOverdNchdEtaSum_vs_Npart");
		// TODO formatGraph(g2, collEnArr, enInd);
		delete g2;
		
		
	} // end of for loop with index centInd
	/// ------ end - plot all npart graphs at once -------------------------------//
	datfile.close();
	return 0;
}

std::string doubToString(Double_t d){
	stringstream stream;
	stream << fixed << setprecision(1) << d;
	return stream.str();

}

void formatGraph(TGraphErrors* g, Double_t collEn_Or_NpartArr[], int en_Or_centInd){
	// for reference:
		// g = new TGraph(9, NpartArrEnByEn, dETdEtaOverNpartBy2SumEnByEn);
	TCanvas* c = new TCanvas(/*"c","A Simple Graph Example",200,10,700,500*/);
	//c->SetLogx(); Npart graphs not in log scale
	TString xlabel;
	TString ylabel;
	string graphText;
	string graphName;
	string imgPathAndName;
	TLatex* t1= new TLatex();
	g->Draw("A*");
	g->SetMarkerSize(2);
	TString snn1("dETdEtaOverNpartBy2Sum_vs_en"); // name of first kind of
									// graph with sqrt(snn) in the x-axis
	TString snn2("dETdEtaOverdNchdEtaSum_vs_en"); // name of second kind of
									// graph with sqrt(snn) in the x-axis
	TString npart1("dETdEtaOverNpartBy2Sum_vs_Npart"); // name of first kind of
												// graph with Npart in the X-axis
	TString npart2("dETdEtaOverdNchdEtaSum_vs_Npart"); // name of second kind of
												// graph with Npart in the X-axis
	TString debug("dETdEtaOverdNchdEtaSum_vs_Npart_Final");// test of graph with
												// interpolated points
	if (g -> GetName()==snn1){
		cout << "Working on npart2********" << endl;
		c -> SetLogx();
		g->SetMarkerStyle(29);
		g->SetMarkerColor(kBlue);
		xlabel = "#sqrt{#it{s}_{NN}} (GeV)";
		ylabel = "#LTd#it{E}_{T}/d#eta#GT/#LT#it{N}_{part}/2#GT (GeV)";
		graphText = centIndToPercent(en_Or_centInd)+" centrality";
		graphName = "dETdEtaOverNpartBy2SumCent" + std::to_string(en_Or_centInd);
		imgPathAndName = 
		"./finalPlots/la_toInterpolate/dETdEtaOverNpartBy2_En/"+graphName+".png";
	}
	else if (g -> GetName()==snn2){
		cout << "Working on npart2********" << endl;
		c -> SetLogx();
		g->SetMarkerStyle(29);
		g->SetMarkerColor(kOrange);
		xlabel = "#sqrt{#it{s}_{NN}} (GeV)";
		ylabel = "#LTd#it{E}_{T}/d#eta#GT/#LTd#it{N}_{ch}/d#eta#GT (GeV)";
		graphText = centIndToPercent(en_Or_centInd)+" centrality";
		graphName = "dETdEtaOverdNchdEtaSumCent" + std::to_string(en_Or_centInd);
		imgPathAndName = 
		"./finalPlots/la_toInterpolate/dETdEtaOverdNchdEta_En/"+graphName+".png";
	}
	else if (g -> GetName()==npart1){
		cout << "Working on npart1********" << endl;
		g->SetMarkerStyle(1);
		//g->SetMarkerSize(1);
		g->SetMarkerColor(kGreen);
		xlabel = "#it{N}_{part}";
		ylabel= "#LTd#it{E}_{T}/d#eta#GT/#LT#it{N}_{part}#GT (GeV)";
		graphText = "#sqrt{#it{s}_{NN}} ="+doubToString(collEn_Or_NpartArr[en_Or_centInd])+" GeV";
		graphName = "dETdEtaOverNpartBy2SumEn" + doubToString(collEn_Or_NpartArr[en_Or_centInd]);
		imgPathAndName = 
		"./finalPlots/la_toInterpolate/dETdEtaOverNpartBy2_Npart/"+graphName+".png";
		
	}
	else if (g -> GetName()==npart2){
		cout << "Working on npart2********" << endl;
		g->SetMarkerStyle(28);
		g->SetMarkerColor(kGreen);
		xlabel = "#it{N}_{part}";
		ylabel= "#LTd#it{E}_{T}/d#eta#GT/#LT#it{N}_{part}#GT (GeV)";
		graphText = "#sqrt{#it{s}_{NN}} ="+doubToString(collEn_Or_NpartArr[en_Or_centInd])+" GeV";
		graphName = "dETdEtaOverNpartBy2SumEn" + doubToString(collEn_Or_NpartArr[en_Or_centInd]);
		imgPathAndName = 
		"./finalPlots/la_toInterpolate/dETdEtaOverNpartBy2_Npart/"+graphName+".png";
	}
	else if (g -> GetName()==debug){
		cout << "Creating debug plot*********** "<< en_Or_centInd << ": " <<
			doubToString(collEn_Or_NpartArr[en_Or_centInd]) << endl;
		g->SetMarkerStyle(1);
		//g->SetMarkerSize(1);
		g->SetMarkerColor(kGreen);
		xlabel = "#it{N}_{part}";
		ylabel= "#LTd#it{E}_{T}/d#eta#GT/#LT#it{N}_{part}#GT (GeV)";
		graphText = "#sqrt{#it{s}_{NN}} ="+doubToString(collEn_Or_NpartArr[en_Or_centInd])+" GeV";
		graphName = "dETdEtaOverNpartBy2SumEn" + doubToString(collEn_Or_NpartArr[en_Or_centInd]);
		imgPathAndName = 
		"./debugPlots/"+graphName+"_debug.png";
	}
	//cout << g->GetName() << endl;
	
	g->GetHistogram()-> SetTitle(0);
	g->GetHistogram()->GetXaxis()-> SetTitle(xlabel);
	g->GetHistogram()->GetXaxis()-> SetTitleOffset(1.05);
	g->GetHistogram()->GetYaxis()-> SetRangeUser(0., 0.050);
	g->GetHistogram()->GetXaxis()-> SetTitleSize(0.04);
	g->GetHistogram()->GetYaxis()-> SetTitle(ylabel);
	g->GetHistogram()->GetYaxis()-> SetTitleSize(0.04);
	g->GetHistogram()->GetYaxis()-> SetTitleOffset(1.05);
	cout << g -> GetName() << " graph formatted" << endl;
	const char* graphTextConstCharPtr = graphText.c_str();// required for TText constructor
	//TText* t1 = new TText(200,.62,graphTextNpartConstCharPtr);
	t1 -> SetNDC(kTRUE);
	t1 -> DrawLatex(0.2,0.8,graphTextConstCharPtr);
	
	//t1 -> SetTextAlign(22);
	t1 -> SetTextSize(0.05);
	//t1 -> DrawText();
	// FIXME t1 -> DrawText(0.2,0.8,graphTextNpartConstCharPtr);
	
				//c -> SaveAs("./fittedPlots/trial1.png");
	TImage *png = TImage::Create();//TODO try to use canvas method instead of png object
	png->FromPad(c);
	const char* imgPathAndNameConstCharPtr1 = imgPathAndName.c_str();
	png->WriteImage(imgPathAndNameConstCharPtr1);
	delete t1;
	delete c;
	delete png;
	return;
}

std::string centIndToPercent(int centInd){
	string centRange;
	if (centInd == 0) centRange = "0-5 %";
	else if (centInd == 1) centRange = "5-10 %";
	else if (centInd == 2) centRange = "10-20 %";
	else if (centInd == 3) centRange = "20-30 %";
	else if (centInd == 4) centRange = "30-40 %";
	else if (centInd == 5) centRange = "40-50 %";
	else if (centInd == 6) centRange = "50-60 %";
	else if (centInd == 7) centRange = "60-70 %";
	else if (centInd == 8) centRange = "70-80 %";
	else centRange = "Error: check centrality!";
	return centRange;	
}

// function to interpolate required points in a graph using a 3-spline
// returns pointer to an array of ET values corresponding to
	// the last four centrality bins: 40-50%, 50-60%, 60-70%, 70-80%
Double_t* interpolateNpartGraph(TGraphErrors* tg, int en){
	Double_t x_interp[4] = {0.};// array to hold the npart values...
							// corresponding to last 4 cents
	Double_t x_interp_err[4] = {0.};
	static Double_t y_interpET[8]			 = {0.}; // array to hold dET/dEta values followed
								// by the corresponding errors FIXME FIXME
	//^ if not static, error:
	//address of stack memory associated with local variable returned
	Double_t y_interp[4]			 = {0.}; // array to hold (dET/dEta)/npart values followed
	Double_t y_interp_err[4]		 = {0.};	// by the corresponding errors FIXME FIXME
	// in order to let the Eval() method use binary search:
	tg->SetBit(TGraph::kIsSortedX);
	for (int i = 5; i<9; i++) // get x-values for points being interpolated
						// i.e., npart values corresponding to last 4 cents
	{
		x_interp[i-5] 		= *getNpartAndErr(collEnArr[en], i);
		x_interp_err[i-5] 	= *(getNpartAndErr(collEnArr[en], i) + 1);
		y_interp[i-5] 		= (tg -> Eval(x_interp[i-5], 0,"S")); // option S implies third-order spline
		// take the percent error in the next point (i-6)? with the 
		 //bigger percent error as the conservative estimate
		 // for the percent error in the above point
		Double_t x; // temp var to hold x-val from tg
		Double_t y; // temp var to hold y-val from tg
		tg -> GetPoint(5+(i-1)/5, x, y); // take the relative error of the 
										//6th bin in the old array as 
										// the relative error of the 6th bin
										// in the new array, and
										// take that of the 7th bin in the old
										// array for the rest of the bins in
										// the new array, to be conservative,
										// since the 7th bin in the old array
										// has the most relative error 
		// find absolute error by multiplying the percent error with the y-value
		y_interp_err[i-5] 	= y_interp[i-5]*(tg->GetErrorY(5+(i-1)/5))/y;
		cout << "***relative error for "<< y_interp[i-5] << ": " 
				<< tg->GetErrorY(5+(i-1)/5)<< "/"
				<< y << "=" << y_interp_err[i-5] << endl;
		// get dET/dEta = (dET/dEta)/npart * npart:
		y_interpET[i-5] 	= y_interp[i-5]*x_interp[i-5];
		// save errors in the same array:
		Double_t x2;
		Double_t y2;
		// assign values to x2 and y2:
		tg -> GetPoint(i+1, x2, y2);
		// using uncertainty propagation formula:
		// error y_interpET[i-1] 	= (tg->GetErrorY(i+1)/y2)*x_interp[i-5];
		y_interpET[i-1]		= TMath::Sqrt((y_interp[i-5]*x_interp_err[i-5])
										* (y_interp[i-5]*x_interp_err[i-5])
										+ (x_interp[i-5]*y_interp_err[i-5])
										* (x_interp[i-5]*y_interp_err[i-5]));
	}
	cout << "x_interp: "; 
	for (int i = 0; i<4; i++){
		cout << *(x_interp+i) << ", ";
	}
	cout << endl;
	cout << "x_interp_err: "; 
	for (int i = 0; i<4; i++){
		cout << *(x_interp_err+i) << ", ";
	}
	cout << endl;
	cout << "y_interp: "; 
	for (int i = 0; i<4; i++){
		cout << *(y_interp+i) << ", ";
	}
	cout << endl;
	cout << "y_interp_err: "; 
	for (int i = 0; i<4; i++){
		cout << *(y_interp_err+i) << ", ";
	}
	cout << endl;
	//cout << "test debug graph iter *************" << endl;
	printInterpolatedGraph(tg, en, x_interp, y_interp, x_interp_err,y_interp_err);
	// so far... created everything that will go into the constructor for the debug graph
	// which is to be created by the function call above
	cout << "En value to pass: " << en << endl;
	return y_interpET;
}

void printInterpolatedGraph(TGraphErrors* tg, int en, Double_t* x_interp, Double_t* y_interp, Double_t* x_interp_err, Double_t* y_interp_err)
{
	// create new tgraph by adding the elements of the old one -2
	// with the newly interpolated points
	// new tgraph constructor:
	TGraphErrors* gr;
	// define arguments that go into testG:
	int sizeFinal; // declaration, defined using the following
	// get x and y axes of the old TGraph:
	//	copy(x_interp, x_interp+size2, xArrFinal+size1-2);
	/* https://stackoverflow.com/questions/12791266/c-concatenate-two-int-arrays-into-one-larger-array
	int * result = new int[size1 + size2];
	copy(arr1, arr1 + size1, result);
	copy(arr2, arr2 + size2, result + size1);
	*/
	Double_t xArr1[tg->GetN()]; // see above comment
	copy(tg->GetX(), tg->GetX()+tg->GetN(), xArr1);
	cout << "xArr1: "; 
	for (int i = 0; i<tg->GetN(); i++){
		cout << *(xArr1+i) << ", ";
	}
	cout << endl;
	Double_t xArr1_err[tg->GetN()];
	copy(tg->GetEX(), tg->GetEX()+tg->GetN(), xArr1_err);
	cout << "xArr1_err: "; 
	for (int i = 0; i<tg->GetN(); i++){
		cout << *(xArr1+i) << ", ";
	}
	cout << endl;
	Double_t yArr1[tg->GetN()];
	copy(tg->GetY(), tg->GetY()+tg->GetN(), yArr1);
	cout << "yArr1: "; 
	for (int i = 0; i<tg->GetN(); i++){
		cout << *(yArr1+i) << ", ";
	}
	cout << endl;
	Double_t yArr1_err[tg->GetN()];
	copy(tg->GetEY(), tg->GetEY()+tg->GetN(), yArr1_err);
	cout << "yArr1_err: "; 
	for (int i = 0; i<tg->GetN(); i++){
		cout << *(yArr1+i) << ", ";
	}
	cout << endl;
	int size1 = tg->GetN(); // size of first array
	//int size2 = sizeof(x_interp)/sizeof(*x_interp); // size of second array
	int size2 = 4; // TODO make this more generic
	// declare array with size size1+size2-2, i.e., the array that can hold
		// all but the last two elements of the first array followed by
		// all the elements of the second array
		// This corresponds to acheiving the regular centralities using
		// the lambda centralities & the regular centralities
	Double_t x = *x_interp;
	sizeFinal = size1+size2-2;
	cout << "size of arr1: " << size1 << " size of arr2: " << size2<< endl;
	cout << "final array size: " << sizeFinal << endl;
	Double_t xArrFinal[sizeFinal];
	Double_t xArrFinal_err[sizeFinal];
	Double_t yArrFinal[sizeFinal];
	Double_t yArrFinal_err[sizeFinal];
	copy(xArr1, xArr1+size1, xArrFinal);
	copy(x_interp, x_interp+size2, xArrFinal+size1-2);
		// HERE TODO TODO TODO TODO TODO TODO
	copy(xArr1_err, xArr1_err+size1, xArrFinal_err);
	copy(x_interp_err, x_interp_err+size2, xArrFinal_err+size1-2);
	copy(yArr1, yArr1+size1, yArrFinal);
	copy(y_interp, y_interp+size2, yArrFinal+size1-2);
	copy(yArr1_err, yArr1_err+size1, yArrFinal_err);
	copy(y_interp_err, y_interp_err+size2, yArrFinal_err+size1-2);
	cout << "xArrFinal: "; 
	for (int i = 0; i<sizeFinal; i++){
		cout << *(xArrFinal+i) << ", ";
	}
	cout << endl;
	cout << "yArrFinal: ";
	for (int i = 0; i<sizeFinal; i++){
		cout << *(yArrFinal+i) << ", ";
	}
	cout << endl;
	gr = new TGraphErrors(sizeFinal, xArrFinal, yArrFinal, xArrFinal_err, yArrFinal_err);
	gr -> SetName("dETdEtaOverdNchdEtaSum_vs_Npart_Final");
	cout << "En test: " << en << endl;
	formatGraph(gr, collEnArr, en);
	delete gr;
	
	//return;
}
