#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

// Defines Course structure
struct Course {
    string courseNumber;
    string courseTitle;
    vector<string> prerequisites;
};

// Defines BST structure
struct TreeNode {
    Course course;
    TreeNode* left;
    TreeNode* right;

    // Constructor for new TreeNode
    TreeNode(Course c) : course(c), left(nullptr), right(nullptr) {}
};

// Inserts a course into BST
TreeNode* insertIntoBST(TreeNode* root, Course course) {
    if (root == nullptr) {
        return new TreeNode(course);
    }

    if (course.courseNumber < root->course.courseNumber) {
        root->left = insertIntoBST(root->left, course);
    }
    else {
        root->right = insertIntoBST(root->right, course);
    }

    return root;
}

// Searches for a course by course number in BST
Course* searchBST(TreeNode* root, const string& courseNumber) {
    if (root == nullptr) {
        return nullptr;
    }

    if (root->course.courseNumber == courseNumber) {
        return &root->course;
    }
    else if (courseNumber < root->course.courseNumber) {
        return searchBST(root->left, courseNumber);
    }
    else {
        return searchBST(root->right, courseNumber);
    }
}

// Splits a string
vector<string> splitString(const string& str, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(str);

    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

// Loads courses from file and inserts into BST
TreeNode* loadCoursesFromFile(const string& filename) {
    TreeNode* root = nullptr;

    cout << "Attempting to open file: " << filename << endl;

    ifstream file(filename);

    if (!file.is_open()) {
        cout << "Error: Unable to open file '" << filename << "'" << endl;
        return nullptr; // returns null pointer if file can't be opened
    }

    string line;
    while (getline(file, line)) {
        vector<string> tokens = splitString(line, ',');

        if (tokens.size() < 2) {
            cout << "Error: Invalid format in line - " << line << endl;
            continue;  // skips invalid lines
        }

        Course course;
        course.courseNumber = tokens[0];
        course.courseTitle = tokens[1];

        for (size_t i = 2; i < tokens.size(); ++i) {
            course.prerequisites.push_back(tokens[i]);
        }

        root = insertIntoBST(root, course);
    }

    file.close();
    return root;
}


// Traverses BST in-order to display courses
void displayCoursesInOrder(TreeNode* root) {
    if (root != nullptr) {
        displayCoursesInOrder(root->left);
        cout << root->course.courseNumber << ": " << root->course.courseTitle << endl;
        if (!root->course.prerequisites.empty()) {
            cout << "Prerequisites: ";
            for (const string& prereq : root->course.prerequisites) {
                cout << prereq << " ";
            }
            cout << endl;
        }
        else {
            cout << "Prerequisites: None" << endl;
        }
        displayCoursesInOrder(root->right);
    }
}

// Displays information for a single course
void displayCourseInfo(TreeNode* root, const string& courseNumber) {
    Course* course = searchBST(root, courseNumber);
    if (course == nullptr) {
        cout << "Course not found." << endl;
    }
    else {
        cout << "Course Number: " << course->courseNumber << endl;
        cout << "Course Title: " << course->courseTitle << endl;
        if (!course->prerequisites.empty()) {
            cout << "Prerequisites: ";
            for (const string& prereq : course->prerequisites) {
                cout << prereq << " ";
            }
            cout << endl;
        }
        else {
            cout << "Prerequisites: None" << endl;
        }
    }
}

// Displays the menu
void displayMenu() {
    cout << "Menu:" << endl;
    cout << "1. Load course data" << endl;
    cout << "2. Print alphanumeric list of all courses" << endl;
    cout << "3. Print course details" << endl;
    cout << "9. Exit" << endl;
}

// Main function for the menu
void mainMenu() {
    TreeNode* root = nullptr;
    bool dataLoaded = false;
    string filename;
    int choice;
    string courseNumber;

    while (true) {
        displayMenu();
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
        case 1:
            // Loads course data
            cout << "Enter the filename containing course data, without quotation marks: ";
            cin.ignore();
            getline(cin, filename);

            // Check for backslashes and handle them properly
            // Replace backslashes if necessary
            for (size_t i = 0; i < filename.length(); ++i) {
                if (filename[i] == '\\') {
                    filename[i] = '/'; // Replacing backslashes with forward slashes
                }
            }

            root = loadCoursesFromFile(filename);  // Try loading the file
            if (root != nullptr) {
                cout << "Course data loaded successfully." << endl;
                dataLoaded = true;
            }
            else {
                cout << "Failed to load course data." << endl;
            }
            break;


        case 2:
            // Prints alphanumeric list of all courses
            if (dataLoaded) {
                cout << "Courses in alphanumeric order:" << endl;
                displayCoursesInOrder(root);
            }
            else {
                cout << "Error: No data loaded." << endl;
            }
            break;

        case 3:
            // Prints course details
            if (dataLoaded) {
                cout << "Enter the course number: ";
                cin >> courseNumber;
                displayCourseInfo(root, courseNumber);
            }
            else {
                cout << "Error: No data loaded." << endl;
            }
            break;

        case 9:
            // Exits program
            cout << "Exiting program." << endl;
            return;

        default:
            cout << "Invalid choice. Please select a valid option." << endl;
        }
    }
}

int main() {
    mainMenu();
    return 0;
}
