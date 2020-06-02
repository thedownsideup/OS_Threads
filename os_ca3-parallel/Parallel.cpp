#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <chrono>
#include <bits/stdc++.h>
#include <map>
#include <algorithm>

using namespace std;
using namespace std::chrono; 

#define BOOKS_LENGTH 12942
#define REVIEWS_LENGTH 517644
#define BOOKS_FILE "Assets/Datasets/books.csv"
#define REVIEWS_FILE "Assets/Datasets/reviews.csv"

#define NUMBER_OF_THREADS 6
pthread_t threads[NUMBER_OF_THREADS];

pthread_mutex_t mutex_books;


class Book {

    public:
    Book(): reviewss(NUMBER_OF_THREADS) {}
    string id;
    string title;
    string genre1;
    string genre2;
    string pages;
    string author;
    double author_avg_rating;
    double score;

    vector<vector<pair<double, int>>> reviewss;

    bool operator<(const Book& other) const {
		return score < other.score;
    }
    
};

map <string, Book> books;
string genre;

void insert_book (vector<string>row) {

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

void read_books(int offset, int len) {
    
    vector<string> row;
    ifstream fin;
    string line;
    string word;

    fin.open(BOOKS_FILE);
    
    fin.seekg(ios::beg);
    for (int i = 0; i < offset; ++i)
        fin.ignore(numeric_limits<streamsize>::max(), '\n');

    for (int i = 0; i < len; i++) {

        if (fin.eof())
            break;
        
        getline(fin, line);
        stringstream s(line);
        row.clear(); 
        
        while (getline(s, word, ','))
            row.push_back(word); 

    pthread_mutex_lock (&mutex_books);
        if (row[2] == genre || row[3] == genre)
            insert_book(row);
    pthread_mutex_unlock (&mutex_books);

    } 
    fin.close();
}

void read_reviews(int offset, int len) {

    int start = offset * len;
    vector<string> row;
    ifstream fin;
    string line;
    string word;

    fin.open(REVIEWS_FILE);
    
    if (offset == 0)
        getline(fin, line);
    
    for (int i = 0; i < start; ++i)
         fin.ignore(numeric_limits<streamsize>::max(), '\n');

    for (int i = 0; i < len; i++) {
        
        if (fin.eof())
            break;

        getline(fin, line);
        stringstream s(line); 

        row.clear();
        while (getline(s, word, ','))
            row.push_back(word); 

        map<string, Book>::iterator it;
        it = books.find(row[0]);
      
        if (it != books.end())
            it->second.reviewss[offset].push_back(make_pair(stod(row[1]), stoi(row[2])));
    }

    fin.close();

}

void calculate_scores () {

    double score;
    for (map<string, Book>::iterator it = books.begin(); it != books.end(); it++) {
        Book book = it->second;
        double rate = 0;
        int likes = 0;
        for(int j = 0; j < book.reviewss.size(); j++){
            for (int k = 0; k < book.reviewss[j].size(); k++){
                rate += book.reviewss[j][k].first * book.reviewss[j][k].second;
                likes += book.reviewss[j][k].second;
            }
        }
        it->second.score = 0.1 * (book.author_avg_rating + rate/likes);
    }
}

void get_best () {
    vector<Book> books_v;

    for (map<string, Book>::iterator it = books.begin(); it != books.end(); ++it)
        books_v.push_back(it->second);

    Book best = *max_element(books_v.begin(), books_v.end());

    cout << "id: " << best.id << endl;
    cout << "Title: " << best.title << endl;
    cout << "Genres: " << best.genre1 << ", " << best.genre2 << endl;
    cout << "Number of Pages: " << best.pages << endl;
    cout << "Author: " << best.author << endl;
    cout << "Average Rating: " << best.author_avg_rating << endl;
}


void* thread_books (void* arg) {

    long offset = (long)arg;
	int len = BOOKS_LENGTH / NUMBER_OF_THREADS;
	int start = offset * len;

    read_books(start, len);
 
	pthread_exit(NULL);
}

void* thread_reviews (void* arg) {

    long offset = (long)arg;
	int len = REVIEWS_LENGTH / NUMBER_OF_THREADS;
    
    read_reviews(offset, len);
 
	pthread_exit(NULL);
}

int main (int argc, char *argv[]) {
    void *status;

    if(argc < 2){
        cout << "ERROR: No Genre Provided\n";
        exit(1);
    }

    genre = argv[1];
    
    pthread_mutex_init(&mutex_books, NULL);

    auto start = high_resolution_clock::now();

	for(long i = 0; i < NUMBER_OF_THREADS; i++)
		pthread_create(&threads[i], NULL, thread_books, (void*)i); 

    // wait for other threads
    for (long tid = 0; tid < NUMBER_OF_THREADS; tid++)
		pthread_join(threads[tid], &status);

    for(long i = 0; i < NUMBER_OF_THREADS; i++)
		pthread_create(&threads[i], NULL, thread_reviews, (void*)i);

    for (long tid = 0; tid < NUMBER_OF_THREADS; tid++)
		pthread_join(threads[tid], &status);
    
    calculate_scores();
    get_best();

    cout << books.size() << endl;

    auto stop = high_resolution_clock::now(); 
    auto duration = duration_cast<microseconds>(stop - start); 
    cout << duration.count() << "ms" << endl;

	/* Last thing that main() should do */
    pthread_mutex_destroy(&mutex_books);
	pthread_exit(NULL);

    return 0;
}
