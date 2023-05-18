#include <iostream>
#include <fstream>
#include <ostream>
#include <string> 

#include "transport_catalogue.h"

using namespace std;
using namespace transport_catalogue;

int main() {

	std::ifstream in("Test.txt");

	std::ofstream out;
	out.open("hello.txt");
	
	StartCatalogue(in, out);

	out.close();

}