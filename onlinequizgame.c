#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_QUESTIONS 5
#define MAX_OPTIONS 4
#define MAX_USERS 10
#define STR_LEN 50
#define DB_FILE "users.dat"

typedef struct {
    char question[256];
    char options[MAX_OPTIONS][100];
    int correctAnswer;
} QuizQuestion;

typedef struct {
    char username[STR_LEN];
    char password[STR_LEN];
    int topScore;
} User;

// --- Function Prototypes ---
void clearInputBuffer();
void getSecureString(char *buffer, int size);
int getMenuChoice();
void loadUsers(User *users, int *totalUsers);
void saveUsers(const User *users, int totalUsers);
void registerUser(User *users, int *totalUsers);
int loginUser(const User *users, int totalUsers);
void userDashboard(User *user, User *allUsers, int totalUsers);
void startQuiz(User *user);
void viewLeaderboard(const User *users, int totalUsers);

int main() {
    User users[MAX_USERS] = {0};
    int totalUsers = 0;
    int choice;

    loadUsers(users, &totalUsers);

    do {
        printf("\n======= QUIZ SYSTEM v2.0 =======\n");
        printf("1. Register\n2. Login\n3. Exit\nSelection: ");
        choice = getMenuChoice();

        switch (choice) {
            case 1: registerUser(users, &totalUsers); break;
            case 2: {
                int userIdx = loginUser(users, totalUsers);
                if (userIdx != -1) userDashboard(&users[userIdx], users, totalUsers);
                break;
            }
            case 3: printf("Goodbye!\n"); break;
            default: printf("Invalid selection.\n");
        }
    } while (choice != 3);
    return 0;
}

void userDashboard(User *user, User *allUsers, int totalUsers) {
    int choice;
    do {
        printf("\n------- WELCOME, %s -------\n", user->username);
        printf("1. Play Quiz (Choose Subject)\n2. View Leaderboard\n3. Logout\nSelection: ");
        choice = getMenuChoice();

        switch (choice) {
            case 1: startQuiz(user); saveUsers(allUsers, totalUsers); break;
            case 2: viewLeaderboard(allUsers, totalUsers); break;
            case 3: printf("Logged out.\n"); break;
        }
    } while (choice != 3);
}

void startQuiz(User *user) {
    // Subject-specific Question Banks
    QuizQuestion mathBank[5] = {
        {"What is 15 * 6?", {"80", "90", "100", "75"}, 1},
        {"Value of Pi (approx)?", {"3.12", "3.16", "3.14", "3.18"}, 2},
        {"Square root of 144?", {"10", "11", "12", "14"}, 2},
        {"What is 7 + 8 * 2?", {"23", "30", "22", "21"}, 0},
        {"A triangle with 90 degree angle is?", {"Obtuse", "Acute", "Right-angled", "Scalene"}, 2}
    };

    QuizQuestion scienceBank[5] = {
        {"Chemical symbol for Water?", {"O2", "CO2", "H2O", "HO2"}, 2},
        {"Which planet is closest to the Sun?", {"Venus", "Earth", "Mercury", "Mars"}, 2},
        {"Boiling point of water?", {"90 C", "100 C", "110 C", "120 C"}, 1},
        {"Gas we breathe in to survive?", {"Carbon Dioxide", "Nitrogen", "Oxygen", "Argon"}, 2},
        {"The 'Powerhouse of the cell' is?", {"Nucleus", "Ribosome", "Mitochondria", "Cytoplasm"}, 2}
    };

    QuizQuestion gkBank[5] = {
        {"Who wrote the National Anthem of India?", {"Tagore", "Gandhi", "Nehru", "Azad"}, 0},
        {"Which is the largest ocean?", {"Atlantic", "Indian", "Arctic", "Pacific"}, 3},
        {"The Statue of Liberty is in?", {"London", "Paris", "New York", "Berlin"}, 2},
        {"Currency of UK?", {"Dollar", "Euro", "Pound", "Yen"}, 2},
        {"Smallest country in the world?", {"Monaco", "Vatican City", "Malta", "Maldives"}, 1}
    };

    printf("\nCHOOSE YOUR SUBJECT:\n1. Mathematics\n2. Science\n3. General Knowledge\nSelection: ");
    int sub = getMenuChoice();
    QuizQuestion *currentBank;

    if (sub == 1) currentBank = mathBank;
    else if (sub == 2) currentBank = scienceBank;
    else if (sub == 3) currentBank = gkBank;
    else { printf("Invalid choice!\n"); return; }

    int score = 0;
    for (int i = 0; i < MAX_QUESTIONS; i++) {
        printf("\nQ%d: %s\n", i + 1, currentBank[i].question);
        for (int j = 0; j < MAX_OPTIONS; j++) printf("  %d. %s\n", j + 1, currentBank[i].options[j]);
        printf("Answer: ");
        if (getMenuChoice() - 1 == currentBank[i].correctAnswer) {
            printf("Correct!\n");
            score++;
        } else printf("Wrong!\n");
    }

    printf("\nQuiz Finished! Score: %d/%d\n", score, MAX_QUESTIONS);
    if (score > user->topScore) user->topScore = score;
}

// --- Utility Functions (Same as before) ---

void clearInputBuffer() { int c; while ((c = getchar()) != '\n' && c != EOF); }
void getSecureString(char *buffer, int size) { fgets(buffer, size, stdin); buffer[strcspn(buffer, "\n")] = 0; }
int getMenuChoice() { int val; if (scanf("%d", &val) != 1) { clearInputBuffer(); return -1; } clearInputBuffer(); return val; }
void loadUsers(User *users, int *totalUsers) { FILE *file = fopen(DB_FILE, "rb"); if (!file) return; *totalUsers = fread(users, sizeof(User), MAX_USERS, file); fclose(file); }
void saveUsers(const User *users, int totalUsers) { FILE *file = fopen(DB_FILE, "wb"); if (!file) return; fwrite(users, sizeof(User), totalUsers, file); fclose(file); }
void registerUser(User *users, int *totalUsers) { if (*totalUsers >= MAX_USERS) return; printf("Username: "); getSecureString(users[*totalUsers].username, STR_LEN); printf("Password: "); getSecureString(users[*totalUsers].password, STR_LEN); users[*totalUsers].topScore = 0; (*totalUsers)++; saveUsers(users, *totalUsers); printf("Done!\n"); }
int loginUser(const User *users, int totalUsers) { char uName[STR_LEN], pWord[STR_LEN]; printf("Username: "); getSecureString(uName, STR_LEN); printf("Password: "); getSecureString(pWord, STR_LEN); for (int i = 0; i < totalUsers; i++) { if (strcmp(users[i].username, uName) == 0 && strcmp(users[i].password, pWord) == 0) return i; } return -1; }
void viewLeaderboard(const User *users, int totalUsers) { printf("\n%-20s | %-10s\n", "PLAYER", "BEST SCORE"); printf("------------------------------------\n"); for (int i = 0; i < totalUsers; i++) printf("%-20s | %-10d\n", users[i].username, users[i].topScore); }