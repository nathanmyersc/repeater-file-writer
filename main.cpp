#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <cmath>
#include <chrono>
#include <cctype> // For tolower()
#include <limits>
using namespace std;

const unsigned long long BLOCK_SIZE = 500 * 1024 * 1024; // 500 MB block size
unsigned long long TOTAL_BYTES; // Updated dynamically
const unsigned long long MAX_THREADS = 8;

atomic<unsigned long long> position(0);
mutex position_mutex;
mutex queue_mutex;
condition_variable cv;
queue<pair<unsigned long long, unsigned long long>> task_queue;
atomic<bool> isDone(false); // Flag to signal threads to exit

void writeFile(const string& intention, const string& filename) {
    ofstream ofs(filename, ios::binary);
    while (true) {
        unique_lock<mutex> lock(queue_mutex);
        cv.wait(lock, []{ return !task_queue.empty() || isDone.load(); });

        if (isDone.load() && task_queue.empty()) {
            break; // Exit the loop if the flag is set and the queue is empty
        }

        if (!task_queue.empty()) {
            pair<unsigned long long, unsigned long long> task = task_queue.front();
            task_queue.pop();
            lock.unlock();

            unsigned long long start = task.first;
            unsigned long long end = start + task.second;

            if (start >= TOTAL_BYTES) {
                continue;
            }

            if (end > TOTAL_BYTES) {
                end = TOTAL_BYTES;
            }

            // Calculate repetitions based on block size and intention size
            unsigned long long blockSizeRepetitions = BLOCK_SIZE / intention.size();
            unsigned long long remainingBytes = end - start;
            unsigned long long repetitions = min(blockSizeRepetitions, (unsigned long long)ceil((double)remainingBytes / intention.size()));

            ofs.seekp(start); // Move the file pointer to the specified position
            for (unsigned long long i = 0; i < repetitions && start < TOTAL_BYTES; ++i) {
                ofs << intention << " ";
                if(i%10 == 0)
                    ofs << "\n";
                ++start;
            }
        } else {
            lock.unlock();
        }
    }
}

void checkTaskQueue() {
    while (true) {
        if (task_queue.empty()) {
            isDone.store(true);
            break;
        }
        this_thread::sleep_for(chrono::milliseconds(100)); // Sleep for 100 milliseconds
    }
}

int main(int argc, char **argv) {
    string intention, filename;
    size_t numThreads;

    cout << "Intention Repeater File Writer v1.0" << endl;
    cout << "By Anthro Teacher (AnthroHeart) aka Thomas Sweet" << endl << endl;

    bool notDecided = true;

    while (notDecided)
    {
        startofloop:;
        cout << "Intention: ";
        getline(cin, intention);
        cout << "How many times do you want to repeat the intention? ";
        unsigned long long repetitions;
        cin >> repetitions;
        TOTAL_BYTES = intention.size() * repetitions; // Calculate total file size
        cout << "Total file size will be approximately: " << (double)TOTAL_BYTES / (1024 * 1024) << " megabytes." << endl;

        cout << "Do you want to proceed with writing to file (yes/no)? ";
        string response;
        cin >> response;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        for (char &c : response) {
            c = tolower(c);
        }
        if (response == "yes" || response == "y") {
            notDecided = false;
        } else if (response == "no" || response == "n") {
           goto startofloop;;
        } else {
            cerr << "Invalid response. Please enter 'yes' or 'no'." << endl;
            goto startofloop;; // Repeat the loop to ask again
        }
    }

    cout << "Filename Base: ";
    cin >> filename;

    // Calculate number of blocks
    unsigned long long numBlocks = ceil((double)TOTAL_BYTES / BLOCK_SIZE);

    // Populate task queue
    unsigned long long pos = 0;
    for (unsigned long long i = 0; i < numBlocks; ++i) {
        unsigned long long blockSize = min(BLOCK_SIZE, TOTAL_BYTES - pos);
        task_queue.push(make_pair(pos, blockSize));
        pos += blockSize;
    }

    // Calculate number of threads based on the number of blocks
    numThreads = min((size_t)numBlocks, MAX_THREADS);

    cout << "Number of threads: " << numThreads << endl;

    // Start the task queue checking thread
    thread checkThread(checkTaskQueue);

    vector<thread> threads;

    for (size_t i = 0; i < numThreads; ++i) {
        threads.emplace_back(writeFile, intention, filename + ".txt");
    }

    // Notify threads to start working
    cv.notify_all();

    // Wait for threads to finish
    for (auto& t : threads) {
        t.join();
    }

    // Wait for the task queue checking thread to finish
    checkThread.join();

    cout << "INTENTION REPEATED TO " << filename + ".txt" << endl;

    return 0;
}
