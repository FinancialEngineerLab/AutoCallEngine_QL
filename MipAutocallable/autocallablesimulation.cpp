#include <ql/quantlib.hpp>
#include <autocallablesimulation.hpp>
#include <autocallablepathpricer.hpp>

using namespace QuantLib;

AutocallableSimulation::AutocallableSimulation(boost::shared_ptr<Quote> underlying,	
	boost::shared_ptr<YieldTermStructure> qTermStructure,
	boost::shared_ptr<YieldTermStructure> bondTermStructure,
	boost::shared_ptr<YieldTermStructure> OISTermStructure,
	boost::shared_ptr<BlackVolTermStructure> volatility,
	Time maturity,
	Real strike,
	Date settlementDate)
	: underlying_(underlying), qTermStructure_(qTermStructure),	bondTermStructure_(bondTermStructure),
	OISTermStructure_(OISTermStructure), volatility_(volatility), maturity_(maturity), strike_(strike), settlementDate_(settlementDate){
}


void AutocallableSimulation::compute(Size nTimeSteps, Size nSamples, char modelType) {

	Real Price;

	// The Monte Carlo model generates paths, according to the "diffusion process", 
	//using the PathGenerator
	// each path is priced using thePathPricer
	// prices will be accumulated into statisticsAccumulator

	const BigNatural seed = 1234;

	if (modelType == 'B') {

		//B&S model

		std::cout << "\nCalcolo del prezzo con il modello di Black&Scholes...\n" << std::endl;

		boost::shared_ptr<StochasticProcess> BSdiffusion(new BlackScholesMertonProcess(
			Handle<Quote>(underlying_),
			Handle<YieldTermStructure>(qTermStructure_),
			Handle<YieldTermStructure>(OISTermStructure_),
			Handle<BlackVolTermStructure>(volatility_)));

		PseudoRandom::rsg_type rsg = PseudoRandom::make_sequence_generator(nTimeSteps, seed);
		typedef MultiVariate<PseudoRandom>::path_generator_type generator_type;
		boost::shared_ptr<generator_type> BSPathGenerator(new
			generator_type(BSdiffusion, TimeGrid(maturity_,nTimeSteps),//maturity_, nTimeSteps,
				rsg, false));

		/*std::shared_ptr<PathPricer<Path>> foo;
		std::shared_ptr<PathPricer<MultiPath>> bar;
		foo = std::dynamic_pointer_cast<PathPricer<Path>>(bar);*/
		//boost::shared_ptr<PathPricer<Path>> BSPathPricer = boost::dynamic_pointer_cast<PathPricer<Path>>(MyPathPricer);
		
		boost::shared_ptr<PathPricer<MultiPath>> BSPathPricer(
			new AutocallablePathPricer(bondTermStructure_,
				OISTermStructure_,
				maturity_,
				strike_,
				settlementDate_));
		
		Statistics statisticsAccumulator;
		
		MonteCarloModel<MultiVariate, PseudoRandom>
			BS_MCSimulation(BSPathGenerator,
				BSPathPricer,
				statisticsAccumulator,
				false);

		BS_MCSimulation.addSamples(nSamples);

		Price = BS_MCSimulation.sampleAccumulator().mean();
	}
	else if (modelType == 'H') {

		//Heston model

		std::cout << "\nCalcolo del prezzo con il modello di Heston...\n" << std::endl;

		//Heston parameters
		Real v0 = 0.0292;
		Real kappa = 1.13;
		Real theta = 0.191;
		Real sigma = 0.74355254;
		Real rho = -0.58486121;

		boost::shared_ptr<StochasticProcess> Hdiffusion(new HestonProcess(
			Handle<YieldTermStructure>(OISTermStructure_),
			Handle<YieldTermStructure>(qTermStructure_),
			Handle<Quote>(underlying_),
			v0, kappa, theta, sigma, rho));
		
		PseudoRandom::rsg_type rsg = PseudoRandom::make_sequence_generator(Hdiffusion->factors() * nTimeSteps, seed);

		typedef MultiVariate<PseudoRandom>::path_generator_type generator_type;
		boost::shared_ptr<generator_type> HPathGenerator(new generator_type(Hdiffusion, TimeGrid(maturity_, nTimeSteps), rsg, false));
		boost::shared_ptr<PathPricer<MultiPath>> HPathPricer(new AutocallablePathPricer(bondTermStructure_,
			OISTermStructure_,
			maturity_,
			strike_,
			settlementDate_));
			
		Statistics statisticsAccumulator;

		MonteCarloModel<MultiVariate, PseudoRandom>
		Heston_MCSimulation(HPathGenerator,
				HPathPricer,
				statisticsAccumulator,
				false);

		Heston_MCSimulation.addSamples(nSamples);		
		
		Price = Heston_MCSimulation.sampleAccumulator().mean();
				
				/*std::vector<Real> Prices;
				for (Size i = 0; i < nSamples; i++) {
				Sample <MultiPath> sample = HPathGenerator->next();
				const MultiPath& paths = sample.value;
				const Path& stock = paths[0];
				const Path& vols = paths[1];
				Prices.push_back(priceCalculator(stock, settlementDate_, strike_, bondTermStructure_, maturity_, OISTermStructure_));
				Price += Prices.at(i);
				}
				Price = Price / nSamples;*/
	}

	std::cout << " \nQuotazione = " << 1005.32 << std::endl;
	std::cout << " \nPrice = " << Price << std::endl;
	std::cout << " \nErrore = " << abs(1-Price/ 1005.32) * 100 << " % " << std::endl;
}

	