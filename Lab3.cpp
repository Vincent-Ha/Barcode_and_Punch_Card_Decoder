#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <regex>
#include <bitset>
#include <string>
#include <queue>
#include <map>
#include <vector>
using namespace std;

// Class Definitions
// Read_File Class
// Helps read the contents of the Punch Card file
class Read_File {
private:
	ifstream fin;
	string data;
public:
	Read_File() {};
	void readFile(string file_path);
	string getData();
};

// EBDCParser Class
// Processes given raw data into a series of "punch cards" that can be used to be translated
class EBDCParser {
private:
	string rawData;
	queue <map <int, string>> punchCards;
public:
	EBDCParser() : rawData("N/A") {};
	void setRawData(string rD);
	void parseFile();
	queue <map<int, string>> getEbdc_Data();
};

// EBDCDecoder
// Takes one punch card and translates the data into Ascii characters
class EBDCDecoder {
private:
	map <int, string> punchCard;
	map<unsigned long int, char> punch_card_cipher;
	string message;
	const int FIRST_ROWS = 3;
	const int NUMBER_OF_ROWS = 12;
	const int ROW_2 = 4;
public:
	EBDCDecoder();
	void setBarcodeData(map <int, string> bD);
	void decodePunchCard();
	string getMessage();
};

// EBDCControlCenter
// Controls the entire process of reading, processing and translating the data found in the punch card file
class EBDCControlCenter {
private:
	Read_File* punchCardReader;
	EBDCParser* punchCardParser;
	vector <EBDCDecoder*> punchCardDecoders;
	string rawData;
	queue <map <int, string>> barcodeData;
	map <int, string> messages;
	void setRawData(string rD);
	void setBarcodeData(queue <map <int, string>> bD);
	void setMessages(map <int, string> m);
public:
	EBDCControlCenter() : punchCardReader(new Read_File), punchCardParser(new EBDCParser), rawData("N/A") {};
	~EBDCControlCenter();
	void readFile(string file_path);
	void parseFile();
	void decodePunchCard();
	map <int, string> getMessages();
	void printMessages();
};

// UserInterface
// Controls what is displayed on the console screen for the user
class UserInterface {
private:
	EBDCControlCenter* driver;
	map <int, string> messages;
public:
	UserInterface() : driver(new EBDCControlCenter) {};
	~UserInterface();
	void menu(string file_path);
};

// Main Function
int main()
{
	UserInterface uI;
	uI.menu("c:/Users/Vincent Ha/Downloads/PumchCards.txt");
    return 0;
}

// Function Definitions
// Read_File functions
void Read_File::readFile(string file_path) {
	string name, content;
	string entireFile = "";
	fin.open(file_path);
	if (!fin.good()) {
		cerr << "Invalid File Path. Please Reload With Proper File Path." << endl;
		exit(-92);
	}

	cout << "Reading data from the file." << endl;
	while (!fin.eof()) {
		getline(fin, content);
		entireFile += content;
		entireFile += "\n";
		content.clear();
	}
	fin.close();
	data = entireFile;
}

string Read_File::getData() {
	return data;
}

// EBDCParser functions
void EBDCParser::setRawData(string rD) {
	rawData = rD;
}

void EBDCParser::parseFile() {
	cout << "Parsing the file" << endl;
	regex spiltIntoLines("(1|0)+|-+");
	regex lines("-+");
	sregex_token_iterator end;
	int index = 0;
	map <int, string> tempMap;
	for (sregex_token_iterator iter(rawData.begin(), rawData.end(), spiltIntoLines); iter != end; iter++) {
		if (regex_match(static_cast<string>(*iter), lines)) {
			if (tempMap.size() != 0) {
				punchCards.push(tempMap);
				tempMap.clear();
				index = 0;
			}
			continue;
		}
		tempMap.emplace(index, static_cast<string>(*iter));
		index++;
	}
	punchCards.push(tempMap);
}

queue <map <int, string>> EBDCParser::getEbdc_Data() {
	return punchCards;
}

// EBDCDecoder functions
EBDCDecoder::EBDCDecoder() :  message("N/A") {
	char letter = '1';
	bitset <12> tempBitset;
	for (int count1 = 0; count1 <= FIRST_ROWS; count1++) {
		for (int count2 = FIRST_ROWS; count2 < NUMBER_OF_ROWS; count2++) {
			tempBitset.reset();
			if (count1 > 0)
				tempBitset.set(count1 - 1);
			tempBitset.set(count2);

			if (count1 == FIRST_ROWS && count2 == FIRST_ROWS) {
				punch_card_cipher.emplace(tempBitset.to_ulong(), '/');
				continue;
			}

			punch_card_cipher.emplace(tempBitset.to_ulong(), letter);
			if (letter == '9') {
				letter = 'A';
				continue;
			}
			letter++;
		}
	}

	vector <char> fillMap = 
	{ '\0', ':', '#', '@', '\'', '=', '"', '&', '[', '.', '<', '(', '+', '!', '-', ']', '$', '*', ')', ';', '^', '0', '\\', ',', '%', '_', '>', '?' };
	int vectorIndex = 0;
	for (int count1 = 0; count1 <= FIRST_ROWS; count1++) {
		for (int count2 = 0; count2 < NUMBER_OF_ROWS - 2; count2++) {
			tempBitset.reset();
			if (count1 > 0)
				tempBitset.set(count1 - 1);

			if (count2 == 0) {
				punch_card_cipher.emplace(tempBitset.to_ulong(), fillMap[vectorIndex]);
				count2 = ROW_2 - 1;
			}
			else {
				tempBitset.set(count2, 1);
				tempBitset.set(NUMBER_OF_ROWS - 2);
				punch_card_cipher.emplace(tempBitset.to_ulong(), fillMap[vectorIndex]);
			}
			vectorIndex++;
		}
	}
}

void EBDCDecoder::setBarcodeData(map <int, string> bD) {
	punchCard = bD;
}

void EBDCDecoder::decodePunchCard() {
	message.clear();
	bitset <12> location_of_ones;
	for (int stringIndex = 0; stringIndex < punchCard[0].size(); stringIndex++) {
		for (int vectorIndex = 0; vectorIndex < punchCard.size(); vectorIndex++) {
			if (punchCard[vectorIndex][stringIndex] == '1') 
				location_of_ones.set(vectorIndex);
		}

		message += punch_card_cipher[location_of_ones.to_ulong()];
		location_of_ones.reset();
	}
}

string EBDCDecoder::getMessage() {
	return message;
}

// EBDCControlCenter Functions
void EBDCControlCenter::setRawData(string rD) {
	rawData = rD;
}

void EBDCControlCenter::setBarcodeData(queue <map <int, string>> bD) {
	barcodeData = bD;
}

void EBDCControlCenter::setMessages(map <int, string> m) {
	messages = m;
}

EBDCControlCenter::~EBDCControlCenter() {
	delete punchCardReader;
	delete punchCardParser;
	for (int index = 0; index < punchCardDecoders.size(); index++) {
		delete punchCardDecoders[index];
	}
}

void EBDCControlCenter::readFile(string file_path) {
	punchCardReader->readFile(file_path);
	setRawData(punchCardReader->getData());
	punchCardParser->setRawData(punchCardReader->getData());
}

void EBDCControlCenter::parseFile() {
	punchCardParser->parseFile();
	setBarcodeData(punchCardParser->getEbdc_Data());
	queue <map <int, string>> copy = barcodeData;
	while (!copy.empty()) {
		punchCardDecoders.push_back(new EBDCDecoder);
		punchCardDecoders[punchCardDecoders.size() - 1]->setBarcodeData(copy.front());
		copy.pop();
	}
}

void EBDCControlCenter::decodePunchCard() {
	map <int, string> messagesCopy;
	int index = 0;
	for (auto source : punchCardDecoders) {
		source->decodePunchCard();
		messagesCopy.emplace(index, source->getMessage());
		index++;
	}
	setMessages(messagesCopy);
}

map <int, string> EBDCControlCenter::getMessages() {
	return messages;
}

void EBDCControlCenter::printMessages() {
	cout << "Messages" << endl;
	cout << "--------" << endl;
	cout << endl;
	for (auto const& message : messages) {
		cout << "Message " << message.first + 1 << ":" << endl;
		cout << message.second << endl;
		cout << endl;
	}
}

// UserInterface functions
UserInterface::~UserInterface() {
	delete driver;
}

void UserInterface::menu(string file_path) {
	cout << "Welcome to the Barcode Decoding Service!" << endl;
	cout << "----------------------------------------" << endl;
	cout << endl;
	cout << "Progress" << endl;
	cout << "--------" << endl;
	driver->readFile(file_path);
	driver->parseFile();
	cout << "Decoding the Punch Cards" << endl;
	driver->decodePunchCard();
	messages = driver->getMessages();
	cout << "Printing Messages" << endl;
	cout << endl;
	driver->printMessages();
}