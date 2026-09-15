/*
 * Online Quiz Game
 * Language: C++
 * Features: User authentication (signup/login), file-based persistent
 *           question bank, randomized non-repeating questions per session,
 *           real-time scoring, results summary, input validation.
 *
 * Author: Priyanka Kumari
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <limits>

using namespace std;

// ------------------------- Question -------------------------
class Question {
public:
    string text;
    vector<string> options;   // 4 options
    int correctIndex;         // 0-based index of correct option
    string category;

    Question(string t, vector<string> opts, int correct, string cat)
        : text(move(t)), options(move(opts)), correctIndex(correct), category(move(cat)) {}
};

// ------------------------- User -------------------------
class User {
public:
    string username;
    string password; // NOTE: stored in plain text for simplicity of this
                      // console project. For real deployments, hash it
                      // (e.g. with bcrypt) before storing.
    int highScore;

    User(string u, string p, int score = 0)
        : username(move(u)), password(move(p)), highScore(score) {}
};

// ------------------------- Utility: input validation -------------------------
int getValidatedIntInput(int minVal, int maxVal) {
    int choice;
    while (true) {
        cin >> choice;
        if (cin.fail() || choice < minVal || choice > maxVal) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number between "
                 << minVal << " and " << maxVal << ": ";
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return choice;
        }
    }
}

string getNonEmptyLine(const string &prompt) {
    string line;
    while (true) {
        cout << prompt;
        getline(cin, line);
        if (!line.empty()) return line;
        cout << "Input cannot be empty. Try again.\n";
    }
}

// ------------------------- File paths -------------------------
const string USERS_FILE = "users.txt";
const string QUESTIONS_FILE = "questions.txt";

// ------------------------- User persistence -------------------------
vector<User> loadUsers() {
    vector<User> users;
    ifstream file(USERS_FILE);
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string username, password, scoreStr;
        getline(ss, username, '|');
        getline(ss, password, '|');
        getline(ss, scoreStr, '|');
        int score = scoreStr.empty() ? 0 : stoi(scoreStr);
        users.emplace_back(username, password, score);
    }
    return users;
}

void saveUsers(const vector<User> &users) {
    ofstream file(USERS_FILE, ios::trunc);
    for (const auto &u : users) {
        file << u.username << "|" << u.password << "|" << u.highScore << "\n";
    }
}

void updateUserScore(vector<User> &users, const string &username, int newScore) {
    for (auto &u : users) {
        if (u.username == username) {
            if (newScore > u.highScore) u.highScore = newScore;
            break;
        }
    }
    saveUsers(users);
}

// ------------------------- Question bank -------------------------
// Loads questions from questions.txt if present, otherwise falls back to a
// built-in default bank so the game always runs even on first clone.
vector<Question> loadQuestions() {
    vector<Question> questions;
    ifstream file(QUESTIONS_FILE);

    if (file) {
        string line;
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            vector<string> fields;
            string field;
            while (getline(ss, field, '|')) fields.push_back(field);
            // Format: category|question|opt1|opt2|opt3|opt4|correctIndex
            if (fields.size() == 7) {
                string category = fields[0];
                string qtext = fields[1];
                vector<string> opts = {fields[2], fields[3], fields[4], fields[5]};
                int correct = stoi(fields[6]);
                questions.emplace_back(qtext, opts, correct, category);
            }
        }
    }

    if (questions.empty()) {
        // Fallback default bank (used if questions.txt is missing/empty)
        questions.emplace_back("What does HTML stand for?",
            vector<string>{"Hyper Trainer Marking Language", "Hyper Text Markup Language",
                            "Hyper Text Marketing Language", "Hyper Text Markup Leveler"}, 1, "Web");
        questions.emplace_back("Which language runs in a web browser?",
            vector<string>{"Java", "C", "Python", "JavaScript"}, 3, "Web");
        questions.emplace_back("What does CSS stand for?",
            vector<string>{"Cascading Style Sheets", "Computer Style Sheets",
                            "Creative Style Sheets", "Colorful Style Sheets"}, 0, "Web");
        questions.emplace_back("Which of these is a NoSQL database?",
            vector<string>{"MySQL", "PostgreSQL", "MongoDB", "SQLite"}, 2, "Database");
        questions.emplace_back("What is the time complexity of binary search?",
            vector<string>{"O(n)", "O(log n)", "O(n^2)", "O(1)"}, 1, "DSA");
        questions.emplace_back("Which keyword is used to create a class in C++?",
            vector<string>{"struct", "object", "class", "define"}, 2, "C++");
        questions.emplace_back("Which HTTP method is used to update a resource?",
            vector<string>{"GET", "POST", "PUT", "DELETE"}, 2, "Web");
        questions.emplace_back("What does REST stand for?",
            vector<string>{"Representational State Transfer", "Remote State Transfer",
                            "Representational Style Transfer", "Reactive State Transfer"}, 0, "Web");
        questions.emplace_back("Which data structure uses FIFO order?",
            vector<string>{"Stack", "Queue", "Tree", "Graph"}, 1, "DSA");
        questions.emplace_back("Which company develops React.js?",
            vector<string>{"Google", "Microsoft", "Meta (Facebook)", "Amazon"}, 2, "Web");
    }

    return questions;
}

// ------------------------- Subjects/Categories -------------------------
vector<string> getUniqueCategories(const vector<Question> &questions) {
    vector<string> categories;
    for (const auto &q : questions) {
        if (find(categories.begin(), categories.end(), q.category) == categories.end()) {
            categories.push_back(q.category);
        }
    }
    return categories;
}

vector<Question> filterByCategory(const vector<Question> &questions, const string &category) {
    if (category == "All") return questions;
    vector<Question> filtered;
    for (const auto &q : questions) {
        if (q.category == category) filtered.push_back(q);
    }
    return filtered;
}

// Shows a numbered list of subjects (plus an "All" option) and returns the
// chosen category name.
string chooseSubject(const vector<Question> &questions) {
    vector<string> categories = getUniqueCategories(questions);

    cout << "\n===== CHOOSE A SUBJECT =====\n";
    for (size_t i = 0; i < categories.size(); i++) {
        cout << "  " << (i + 1) << ". " << categories[i] << "\n";
    }
    int allOption = (int)categories.size() + 1;
    cout << "  " << allOption << ". All Subjects (Mixed)\n";
    cout << "Choose an option: ";

    int choice = getValidatedIntInput(1, allOption);
    if (choice == allOption) return "All";
    return categories[choice - 1];
}

// ------------------------- Authentication -------------------------
User* signup(vector<User> &users) {
    string username = getNonEmptyLine("Choose a username: ");
    for (auto &u : users) {
        if (u.username == username) {
            cout << "Username already exists. Please login instead.\n";
            return nullptr;
        }
    }
    string password = getNonEmptyLine("Choose a password: ");
    users.emplace_back(username, password);
    saveUsers(users);
    cout << "Signup successful! You can now log in.\n";
    return nullptr;
}

User* login(vector<User> &users) {
    string username = getNonEmptyLine("Username: ");
    string password = getNonEmptyLine("Password: ");
    for (auto &u : users) {
        if (u.username == username && u.password == password) {
            cout << "Login successful. Welcome back, " << username << "!\n";
            return &u;
        }
    }
    cout << "Invalid username or password.\n";
    return nullptr;
}

// ------------------------- Quiz logic -------------------------
int runQuiz(vector<Question> questions, int numQuestions) {
    // Shuffle to avoid repeat order/pattern each session
    random_device rd;
    mt19937 g(rd());
    shuffle(questions.begin(), questions.end(), g);

    numQuestions = min(numQuestions, (int)questions.size());
    int score = 0;

    for (int i = 0; i < numQuestions; i++) {
        const Question &q = questions[i];
        cout << "\nQ" << (i + 1) << ". [" << q.category << "] " << q.text << "\n";
        for (size_t j = 0; j < q.options.size(); j++) {
            cout << "  " << (j + 1) << ". " << q.options[j] << "\n";
        }
        cout << "Your answer (1-" << q.options.size() << "): ";
        int answer = getValidatedIntInput(1, (int)q.options.size());

        if (answer - 1 == q.correctIndex) {
            cout << "Correct!\n";
            score++;
        } else {
            cout << "Wrong. Correct answer: " << q.options[q.correctIndex] << "\n";
        }
    }

    cout << "\n----- Quiz Complete -----\n";
    cout << "Score: " << score << " / " << numQuestions << "\n";
    return score;
}

// ------------------------- Main menu -------------------------
void mainMenu() {
    vector<User> users = loadUsers();
    User *currentUser = nullptr;

    while (!currentUser) {
        cout << "\n===== ONLINE QUIZ GAME =====\n";
        cout << "1. Login\n2. Sign Up\n3. Exit\n";
        cout << "Choose an option: ";
        int choice = getValidatedIntInput(1, 3);

        if (choice == 1) {
            currentUser = login(users);
        } else if (choice == 2) {
            signup(users);
        } else {
            cout << "Goodbye!\n";
            return;
        }
    }

    vector<Question> questions = loadQuestions();

    bool playing = true;
    while (playing) {
        cout << "\n===== MAIN MENU (" << currentUser->username << ") =====\n";
        cout << "1. Start Quiz\n2. View High Score\n3. Logout & Exit\n";
        cout << "Choose an option: ";
        int choice = getValidatedIntInput(1, 3);

        if (choice == 1) {
            string subject = chooseSubject(questions);
            vector<Question> pool = filterByCategory(questions, subject);

            if (pool.empty()) {
                cout << "No questions available for that subject yet.\n";
                continue;
            }

            int available = (int)pool.size();
            cout << "Subject: " << subject << " (" << available << " questions available)\n";
            cout << "How many questions do you want (1-" << available << ")? ";
            int numQ = getValidatedIntInput(1, available);
            int score = runQuiz(pool, numQ);
            updateUserScore(users, currentUser->username, score);
        } else if (choice == 2) {
            cout << "Your high score: " << currentUser->highScore << "\n";
        } else {
            cout << "Thanks for playing, " << currentUser->username << "!\n";
            playing = false;
        }
    }
}

int main() {
    mainMenu();
    return 0;
}
