#include <boost/timer.hpp>
#include <iostream>
#include <ql/quantlib.hpp>
#include <marketdata.hpp>
#include <autocallablesimulation.hpp>

#ifdef BOOST_MSVC
#  include <ql/auto_link.hpp>
#endif

using namespace QuantLib;

// Compute the price of an Autocallable Investment Certificate

int main(int, char*[]) {

	try {

		boost::timer timer;
		std::cout << std::endl;

		//date setting
		Calendar calendar = TARGET();
		DayCounter dayCount = Actual365Fixed();
		Date todaysDate(31, March, 2017);

		Integer fixingDays = 2;
		Date settlementDate = calendar.advance(todaysDate, fixingDays, Days);
		// must be a business day
		settlementDate = calendar.adjust(settlementDate);
		Settings::instance().evaluationDate() = todaysDate;
		todaysDate = Settings::instance().evaluationDate();

		//option input-data		
		Date optionExpiryDate(03, March, 2021);
		Time maturity = dayCount.yearFraction(settlementDate, optionExpiryDate);
		Real strike = 15.08;
		boost::shared_ptr<Quote> underlying(new SimpleQuote(15.35));		

		//discounting curve, dividend curve and volatility term structure
		auto OISTermStructure = MarketData::builddiscountingcurve(settlementDate, fixingDays);
		auto qTermStructure = MarketData::builddividendcurve(settlementDate, fixingDays, OISTermStructure);
		auto varTS = MarketData::buildblackvariancesurface(settlementDate, calendar);
		Volatility sigma = varTS->blackVol(optionExpiryDate, strike);
		const boost::shared_ptr<BlackVolTermStructure> volatility(new BlackConstantVol(settlementDate, calendar, sigma, dayCount));

		//bond discounting curve
		auto bondTermStructure = MarketData::buildbonddiscountingurve(settlementDate, fixingDays);
		
		//Price calculation via Montecarlo simulation
		AutocallableSimulation autocall(underlying, qTermStructure, bondTermStructure, OISTermStructure, volatility, maturity, strike, settlementDate);
		Size nTimeSteps = 10000;
		Size nSamples = 5000;

		/*std::cout << "\nt1_s =" << dayCount.yearFraction(settlementDate, Date(21, February, 2018)) << std::endl;
		std::cout << " t1_e = " << dayCount.yearFraction(settlementDate, Date(27, February, 2018)) << std::endl;
		std::cout << "\nt2_s = "<< dayCount.yearFraction(settlementDate, Date(20, February, 2019)) << std::endl;
		std::cout << " t2_e = "<< dayCount.yearFraction(settlementDate, Date(26, February, 2019)) << std::endl;
		std::cout << "\nt3_s = "<< dayCount.yearFraction(settlementDate, Date(20, February, 2020)) << std::endl;
		std::cout << " t3_e = "<< dayCount.yearFraction(settlementDate, Date(26, February, 2020)) << std::endl;
		std::cout << "\nbar = "<< dayCount.yearFraction(settlementDate, Date(01, March, 2021)) << std::endl;*/

		//model choise
		char modelType;
		bool fails = false;
		do {
			std::cout << "Digita la scelta del modello con cui prezzare:\n\n";
			std::cout << "   -) B per Black&Scholes;\n";
			std::cout << "   -) H per Heston.\n\n";
			std::cin >> modelType;
			modelType = toupper(modelType);
			if ((modelType == 'B') || (modelType == 'H')){
				autocall.compute(nTimeSteps, nSamples, modelType);
			}
			else
			{	fails = true;
				std::cout << "\nNon � stato inserito un dato valido!\nSi prega di riprovare\n" << std::endl;}
		} while (fails);		
		
		//timer
		double seconds = timer.elapsed();
		Integer hours = int(seconds / 3600);
		seconds -= hours * 3600;
		Integer minutes = int(seconds / 60);
		seconds -= minutes * 60;
		std::cout << " \nRun completed in ";
		if (hours > 0)
			std::cout << hours << " h ";
		if (hours > 0 || minutes > 0)
			std::cout << minutes << " m ";
		std::cout << std::fixed << std::setprecision(0)
			<< seconds << " s\n" << std::endl;
			
		return 0;
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	catch (...) {
		std::cerr << "unknown error" << std::endl;
		return 1;
	}
}


