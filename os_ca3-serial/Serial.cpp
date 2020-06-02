#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <chrono>
#include <bits/stdc++.h>
#include <map>

using namespace std;
using namespace std::chrono; 

class Book {

    public:

    string id;
    string title;
    string genre1;
    string genre2;
    string pages;
    string author;
    double author_avg_rating;
    double score;
    vector<pair<double, int>> reviews;

    bool operator<(const Book& other) const {
		return score < other.score;
    }
    
};

map <string, Book> books;

void read_books (string genre) {
    ifstream fin;
    string line;
    vector<string> row;
    string word;

    fin.open("Assets/Datasets/books.csv");
    getline(fin, line);
    while (getline(fin, line)) {
        row.clear();
        
        stringstream s(line); 
        while (getline(s, word, ','))
            row.push_back(word); 
        
        if (row[2] == genre || row[3] == genre) {
            Book newbook;
            newbook.id = row[0];
            newbook.title = row[1];
            newbook.genre1 = row[2];
            newbook.genre2 = row[3];
            newbook.pages = row[4];
            newbook.author = row[5];
            newbook.author_avg_rating = stod(row[6]);
            books.insert(make_pair(row[0], newbook));
        }
    }
}

void read_reviews () {

    ifstream fin;
    string line;
    vector<string> row;
    string word;

    fin.open("Assets/Datasets/reviews.csv");
    getline(fin, line);

    while (getline(fin, line)) {

        row.clear();
        stringstream s(line); 

        while (getline(s, word, ','))
            row.push_back(word); 

        map<string, Book>::iterator it;
        it = books.find(row[0]);
      
        if (it != books.end())
            it->second.reviews.push_back(make_pair(stod(row[1]), stoi(row[2])));
    }
}

void calculate_scores () {

    double score;
    for (map<string, Book>::iterator it = books.begin(); it != books.end(); it++ ) {
        Book book = it->second;
        double rate = 0;
        int likes = 0;
        for(int j = 0; j < book.reviews.size(); j++){
            rate += book.reviews[j].first * book.reviews[j].second;
            likes += book.reviews[j].second;
        }
        it->second.score = 0.1 * (book.author_avg_rating + rate/likes);
    }
}

void get_best () {
    vector<Book> books_v;

    for (map<string, Book>::iterator it = books.begin(); it != books.end(); ++it) { 
        books_v.push_back(it->second);
    }

    Book best = *max_element(books_v.begin(), books_v.end());

    cout << "id: " << best.id << endl;
    cout << "Title: " << best.title << endl;
    cout << "Genres: " << best.genre1 << ", " << best.genre2 << endl;
    cout << "Number of Pages: " << best.pages << endl;
    cout << "Author: " << best.author << endl;
    cout << "Average Rating: " << best.author_avg_rating << endl;
}

int main (int argc, char *argv[]) {

	if (argc < 2) {
        cout << "ERROR: No Genre Provided\n";
        exit(1);
    }

    auto start = high_resolution_clock::now(); 

	read_books(argv[1]);

    auto stop = high_resolution_clock::now(); 
    auto duration = duration_cast<microseconds>(stop - start); 
    cout << "read_books time: " << duration.count() << "ms" << endl;
    start = high_resolution_clock::now(); 

    read_reviews();

    stop = high_resolution_clock::now(); 
    duration = duration_cast<microseconds>(stop - start); 
    cout << "read_reviews time: " << duration.count() << "ms" << endl;
    start = high_resolution_clock::now(); 
    
    calculate_scores();

    stop = high_resolution_clock::now(); 
    duration = duration_cast<microseconds>(stop - start); 
    cout << "caculare_scores time: " << duration.count() << "ms" << endl;
    start = high_resolution_clock::now(); 
    
    get_best();

    stop = high_resolution_clock::now(); 
    duration = duration_cast<microseconds>(stop - start); 
    cout << "get_best time: " << duration.count() << "ms" << endl;
    return 0;
}

