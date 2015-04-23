#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#define _USE_MATH_DEFINES
#define M_PI 3.14159265358979323846
using namespace std;

double degrad(double d) {
	return d * M_PI / 180;
}


// Code below is adapted from http://www.movable-type.co.uk/scripts/latlong.html
// FYI: That website has an applet that computes the haversine distance. 
// It also has a link that will show the locations on a map,
// with a line between them.
double haverdist(double lat1, double longit1, double lat2, double longit2)
{
	double r = 6371;
	double dlat = degrad(lat2 - lat1);
	double dlongit = degrad(longit2 - longit1);
	double a = sin(dlat / 2)*sin(dlat / 2) +
		cos(degrad(lat1))*cos(degrad(lat2))*sin(dlongit / 2)*sin(dlongit / 2);
	double c = 2 * atan2(sqrt(a), sqrt(1 - a));
	return r*c;
}

class trainStopData {
public:
	trainStopData(string id, string name, string lat, string lon) : stop_id(id), stop_name(name), stop_lat(stod(lat)), stop_lon(stod(lon)){

	}
	string get_id() const { return stop_id; }
	string get_stop_name() const { return stop_name; }
	double get_latitude() const { return stop_lat; }
	double get_longitude() const { return stop_lon; }
private:
	string stop_id;
	string stop_name;
	double stop_lat;
	double stop_lon;
};

void read_MTA_file(ifstream& ifs, vector<trainStopData*>& stops) {
	//Just to get rid of the first line
	string format;
	ifs >> format;


	int start;
	string s;
	vector<string> fields;
	string result;
	while (getline(ifs, s)) {
		getline(ifs, s);
		start = 0;
		int pos1 = s.find_first_of(',');
		do{
			string result = s.substr(start, pos1 - start);
			start = pos1 + 2;
			fields.push_back(result);
			pos1 = s.find_first_of(",,", start);
		} while (pos1 != -1);
		stops.push_back(new trainStopData(fields[0], fields[1], fields[2], fields[3]));
		fields.clear();
	}
}

template <class Iterator, class Predicate, class Operator>
int perform_if(const Iterator& itrStart, const Iterator& itrEnd, Predicate pred, Operator op) {
	int count = 0;
	for (Iterator i = itrStart; i != itrEnd; ++i) {
		if (pred(*i)) {
			op(*i);
		}
		count++;
	}
	return count;
}

class isStopOnRoute {
public:
	explicit isStopOnRoute(char c) : route(c) {}
	bool operator()(const trainStopData* stop) {
		string id = stop->get_id();
		if (id[0] == route) {
			return true;
		}
		return false;
	}
private:
	char route;
};

class isSubwayStop {
public:
	isSubwayStop(string id) : stopId(id) {}
	bool operator()(const trainStopData* stop) {
		if (stop->get_id() == stopId) {
			return true;
		}
		else return false;
	}
private:
	string stopId;
};

class isSubwayStopNearX {
public:
	isSubwayStopNearX(double lat, double lon, double d) : latitude(lat), longitude(lon), d(d) {}
	bool operator()(const trainStopData* stop) {
		if (haverdist(latitude, longitude, stop->get_latitude(), stop->get_longitude()) < d) {
			return true;
		}
		else return false;
	}
private:
	double longitude;
	double latitude;
	double d;
};

class printTrainStopInfo {
public:
	void operator()(const trainStopData* stop) {
		cout << "Stop ID: " << stop->get_id() << ", Stop Name: " << stop->get_stop_name() << ", Location: " << stop->get_latitude() << "," << stop->get_longitude() << endl;
	}
private:

};

template<class Iterator, class Predicate>
Iterator select_item(Iterator& begin, Iterator& end, Predicate pred) {
	Iterator target = end;
	for (Iterator i = begin; i != end; i++) {
		if (pred(*i)) {
			target = i;
		}
	}
	return target;
}

class closest_stop {
public:
	closest_stop(double lat, double lon) : latitude(lat), longitude(lon), shortest_dist(numeric_limits<double>::max()) {}
	bool operator()(trainStopData* stop) {
		if (haverdist(stop->get_latitude(), stop->get_longitude(), latitude, longitude) < shortest_dist) {
			shortest_dist = haverdist(stop->get_latitude(), stop->get_longitude(), latitude, longitude);
			return true;
		}
		else return false;
	}
private:
	double latitude;
	double longitude;
	double shortest_dist;
};

void trainStopMenu(vector<trainStopData*>& stops) {
	cout << "Welcome to the MTA train stop interface! Type 'route' for information about a specific subway route. Type 'stop' for information about a specific subway stop. Type 'distance' for all subway stops within a certain distance. Type 'closest' to find your nearest subway stop. Type 'quit' to quit!" << endl;
	string response;
	char route;
	string id;
	double numLat;
	double numLon;
	double d;
	cin >> response;
	while (response != "quit") {
		if (response == "route") {
			cout << "What route would you like more information about?" << endl;
			cin >> route;
			perform_if(stops.begin(), stops.end(), isStopOnRoute(route), printTrainStopInfo());
		}
		else if (response == "stop") {
			cout << "What subway stop would you like more information about?" << endl;
			string id;
			cin >> id;
			perform_if(stops.begin(), stops.end(), isSubwayStop(id), printTrainStopInfo());
		}
		else if (response == "distance") {
			cout << "What is your current latitude?" << endl;
			cin >> numLat;
			cout << "What is your current longitude?" << endl;
			cin >> numLon;
			cout << "What is the maximum distance, in kilometers, a stop can be from your location?" << endl;
			cin >> d;
			isSubwayStopNearX closeBy(numLat, numLon, d);
			perform_if(stops.begin(), stops.end(), closeBy, printTrainStopInfo());
		}
		else if (response == "closest") {
			cout << "What is your current latitude?" << endl;
			cin >> numLat;
			cout << "What is your current longitude?" << endl;
			cin >> numLon;
			printTrainStopInfo print;
			print(*select_item(stops.begin(), stops.end(), closest_stop(numLat, numLon)));
		}
		else cout << "That is not a valid command." << endl;
		cout << "Type another command if you wish to continue." << endl;
		cin >> response;
	}
	cout << "Thank you for using the MTA train stop interface!" << endl;
}

int main() {
	vector<trainStopData*> stops;
	ifstream ifs("mta_info.txt");
	read_MTA_file(ifs, stops);
	trainStopMenu(stops);
	system("pause");
}