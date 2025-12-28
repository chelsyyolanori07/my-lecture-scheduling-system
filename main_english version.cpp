#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <limits>

using namespace std;

const int MAX_SLOTS = 10;
const int MAX_COURSES_DB = 100;
const int MAX_STUDENTS = 50;

// Structure for time sessions (according to credits)
struct TimeSession {
    string startTime;
    string endTime;
    int durationMinutes; // in minutes
};

// Available time sessions list (07.30 - 15.30, total 10 credits)
// 1 credit = 45 minutes, with breaks at 09.45-10.00 and 13.00-13.15
const TimeSession SESSION_TIMES[] = {
    // 1 Credit Session (45 minutes)
    {"07.30", "08.15", 45},   // Slot 1
    {"08.15", "09.00", 45},   // Slot 2
    {"09.00", "09.45", 45},   // Slot 3
    {"10.00", "10.45", 45},   // Slot 4 (after break)
    {"10.45", "11.30", 45},   // Slot 5
    {"11.30", "12.15", 45},   // Slot 6
    {"12.15", "13.00", 45},   // Slot 7
    {"13.15", "14.00", 45},   // Slot 8 (after break)
    {"14.00", "14.45", 45},   // Slot 9
    {"14.45", "15.30", 45},   // Slot 10

    // 2 Credits Session (90 minutes)
    {"07.30", "09.00", 90},   // Slot 1-2
    {"08.15", "09.45", 90},   // Slot 2-3
    {"10.00", "11.30", 90},   // Slot 4-5
    {"10.45", "12.15", 90},   // Slot 5-6
    {"11.30", "13.00", 90},   // Slot 6-7
    {"13.15", "14.45", 90},   // Slot 8-9
    {"14.00", "15.30", 90},   // Slot 9-10

    // 3 Credits Session (135 minutes)
    {"07.30", "09.45", 135},  // Slot 1-3
    {"10.00", "12.15", 135},  // Slot 4-6
    {"10.45", "13.00", 135},  // Slot 5-7
    {"13.15", "15.30", 135},  // Slot 8-10
};
const int NUMBER_OF_SESSIONS = 21;

// Available days list
const string DAYS[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday"};
const int NUMBER_OF_DAYS = 5;

struct CourseMaster {
    string courseName;
    string startTime;
    string endTime;
    string room;
    int credits;
    string lecturer;
    string day; // Stores day (Monday-Friday)
    int sessionIndex; // Stores selected session index
};

struct Student {
    string name;
    string studentID;
    string schedule[MAX_SLOTS + 1];
    string room[MAX_SLOTS + 1];
    string startTime[MAX_SLOTS + 1];
    string endTime[MAX_SLOTS + 1];
    string day[MAX_SLOTS + 1]; // Stores day for each slot
};

CourseMaster courseDatabase[MAX_COURSES_DB];
int numberOfCoursesRegistered = 0;

Student studentDatabase[MAX_STUDENTS];
int numberOfStudentsRegistered = 0;

// --- HELPER FUNCTIONS ---
string trim(string str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

void printHeader(string title) {
    int length = title.length() + 4;
    cout << "\n";
    cout << string(length, '=') << endl;
    cout << "  " << title << endl;
    cout << string(length, '=') << endl;
}

void printLine() {
    cout << "------------------------------------------------------------" << endl;
}

void printLineDynamic(int length) {
    cout << string(length, '-') << endl;
}

bool searchCourse(string searchName, CourseMaster &result) {
    for (int i = 0; i < numberOfCoursesRegistered; i++) {
        if (courseDatabase[i].courseName == searchName) {
            result = courseDatabase[i];
            return true;
        }
    }
    return false;
}

bool searchStudentByName(string searchName, Student &result) {
    for (int i = 0; i < numberOfStudentsRegistered; i++) {
        if (studentDatabase[i].name == searchName) {
            result = studentDatabase[i];
            return true;
        }
    }
    return false;
}

// Function for numeric input validation
int inputInteger(string prompt, int min = INT_MIN, int max = INT_MAX) {
    int value;
    while (true) {
        cout << prompt;
        cin >> value;

        if (cin.fail()) {
            cout << "   [!] Input must be a number! Please try again.\n" << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (value < min || value > max) {
            cout << "   [!] Input must be between " << min << " and " << max << "! Please try again.\n" << endl;
            continue;
        }

        return value;
    }
}

// Function to display day options
void displayDayOptions() {
    cout << "\nAvailable Class Days:" << endl;
    cout << "-------------------" << endl;
    for (int i = 0; i < NUMBER_OF_DAYS; i++) {
        cout << (i+1) << ". " << DAYS[i] << endl;
    }
    cout << "-------------------" << endl;
    cout << "Note: Saturday and Sunday are off (cannot be selected)" << endl;
    cout << "-------------------" << endl;
}

// Function to display session time options based on credits
void displaySessionOptions(int credits) {
    cout << "\nSession Time Options (based on " << credits << " credits):" << endl;
    cout << "------------------------------------------------" << endl;

    int requiredDuration = credits * 45; // 1 credit = 45 minutes
    int counter = 1;

    for (int i = 0; i < NUMBER_OF_SESSIONS; i++) {
        if (SESSION_TIMES[i].durationMinutes == requiredDuration) {
            cout << counter << ". " << SESSION_TIMES[i].startTime << " - "
                 << SESSION_TIMES[i].endTime << " (" << credits << " credits)" << endl;
            counter++;
        }
    }

    if (counter == 1) {
        cout << "No sessions available for " << credits << " credits." << endl;
    }
    cout << "------------------------------------------------" << endl;
}

// Function to get session based on selection
bool getSessionBasedOnChoice(int credits, int choice, TimeSession &session) {
    int requiredDuration = credits * 45;
    int counter = 1;

    for (int i = 0; i < NUMBER_OF_SESSIONS; i++) {
        if (SESSION_TIMES[i].durationMinutes == requiredDuration) {
            if (counter == choice) {
                session = SESSION_TIMES[i];
                return true;
            }
            counter++;
        }
    }

    return false;
}

// Function to select day
string selectDay() {
    int dayChoice;
    
    do {
        displayDayOptions();
        dayChoice = inputInteger("Select day (1-5): ", 1, NUMBER_OF_DAYS);
    } while (dayChoice < 1 || dayChoice > NUMBER_OF_DAYS);
    
    return DAYS[dayChoice - 1];
}

// Function to check student schedule conflict
bool checkScheduleConflict(Student student, string newDay, string newStartTime, string newEndTime, string courseToCheck = "") {
    // Convert time to minutes for comparison
    auto timeToMinutes = [](string time) {
        int hours = stoi(time.substr(0, 2));
        int minutes = stoi(time.substr(3, 2));
        return hours * 60 + minutes;
    };
    
    int newStart = timeToMinutes(newStartTime);
    int newEnd = timeToMinutes(newEndTime);
    
    // Check each schedule slot
    for (int s = 1; s <= MAX_SLOTS; s++) {
        if (student.schedule[s] != "Empty" && student.schedule[s] != "-" && student.schedule[s] != courseToCheck) {
            // Check if same day
            if (student.day[s] == newDay) {
                int existingStart = timeToMinutes(student.startTime[s]);
                int existingEnd = timeToMinutes(student.endTime[s]);
                
                // Check time conflict
                if ((newStart < existingEnd && newEnd > existingStart)) {
                    return true; // There is a conflict
                }
            }
        }
    }
    
    return false; // No conflict
}

// --- FUNCTIONS TO LOAD DATA FROM FILE ---
void loadCourseData() {
    ifstream file("courses.txt");
    string line;
    numberOfCoursesRegistered = 0;

    if (!file.is_open()) {
        cout << "File courses.txt not found, will create new." << endl;
        return;
    }

    while (getline(file, line) && numberOfCoursesRegistered < MAX_COURSES_DB) {
        if (trim(line) == "") continue;

        vector<string> parts;
        size_t start = 0;
        size_t end = 0;

        while ((end = line.find('|', start)) != string::npos) {
            string part = line.substr(start, end - start);
            parts.push_back(trim(part));
            start = end + 1;
        }

        if (start < line.length()) {
            parts.push_back(trim(line.substr(start)));
        }

        if (parts.size() >= 8) { // Now has 8 parts with day and sessionIndex
            courseDatabase[numberOfCoursesRegistered].courseName = parts[0];
            courseDatabase[numberOfCoursesRegistered].startTime = parts[1];
            courseDatabase[numberOfCoursesRegistered].endTime = parts[2];
            courseDatabase[numberOfCoursesRegistered].room = parts[3];
            try {
                courseDatabase[numberOfCoursesRegistered].credits = stoi(parts[4]);
                courseDatabase[numberOfCoursesRegistered].sessionIndex = stoi(parts[7]);
            } catch (...) {
                courseDatabase[numberOfCoursesRegistered].credits = 3;
                courseDatabase[numberOfCoursesRegistered].sessionIndex = -1;
            }
            courseDatabase[numberOfCoursesRegistered].lecturer = parts[5];
            courseDatabase[numberOfCoursesRegistered].day = parts[6];

            numberOfCoursesRegistered++;
        }
    }
    file.close();
}

void loadStudentData() {
    ifstream file("students.txt");
    string line;
    numberOfStudentsRegistered = 0;

    if (!file.is_open()) {
        cout << "File students.txt not found, will create new." << endl;
        return;
    }

    // Reset student database
    for (int i = 0; i < MAX_STUDENTS; i++) {
        studentDatabase[i].name = "";
        studentDatabase[i].studentID = "";
        for (int j = 1; j <= MAX_SLOTS; j++) {
            studentDatabase[i].schedule[j] = "Empty";
            studentDatabase[i].room[j] = "-";
            studentDatabase[i].startTime[j] = "-";
            studentDatabase[i].endTime[j] = "-";
            studentDatabase[i].day[j] = "-";
        }
    }

    while (getline(file, line) && numberOfStudentsRegistered < MAX_STUDENTS) {
        if (trim(line) == "") continue;

        vector<string> parts;
        size_t start = 0;
        size_t end = 0;

        for (int i = 0; i < 2; i++) {
            end = line.find('|', start);
            if (end == string::npos) break;
            parts.push_back(trim(line.substr(start, end - start)));
            start = end + 1;
        }

        if (parts.size() < 2) continue;

        Student s;
        s.name = parts[0];
        s.studentID = parts[1];

        for (int k = 1; k <= MAX_SLOTS; k++) {
            s.schedule[k] = "Empty";
            s.room[k] = "-";
            s.startTime[k] = "-";
            s.endTime[k] = "-";
            s.day[k] = "-";
        }

        string remaining = line.substr(start);
        vector<string> courseList;
        size_t courseStart = 0;

        while ((end = remaining.find('|', courseStart)) != string::npos) {
            string course = trim(remaining.substr(courseStart, end - courseStart));
            if (!course.empty()) {
                courseList.push_back(course);
            }
            courseStart = end + 1;
        }

        string lastCourse = trim(remaining.substr(courseStart));
        if (!lastCourse.empty()) {
            courseList.push_back(lastCourse);
        }

        int slotIndex = 1;
        for (const string& courseName : courseList) {
            if (slotIndex > MAX_SLOTS) break;

            CourseMaster cm;
            if (searchCourse(courseName, cm)) {
                s.schedule[slotIndex] = courseName;
                s.room[slotIndex] = cm.room;
                s.startTime[slotIndex] = cm.startTime;
                s.endTime[slotIndex] = cm.endTime;
                s.day[slotIndex] = cm.day;
                slotIndex++;
            }
        }

        studentDatabase[numberOfStudentsRegistered] = s;
        numberOfStudentsRegistered++;
    }
    file.close();
}

// --- FUNCTIONS FOR NEW DATA INPUT ---
void addNewCourse() {
    string courseName, room, lecturer, day;
    int credits, sessionChoice;

    printHeader("ADD NEW COURSE");

    cout << "\nEnter Course Name      : ";
    getline(cin, courseName);

    // Input credits with validation
    credits = inputInteger("Enter Credits (1-3)     : ", 1, 3);

    // Select day
    day = selectDay();
    cout << "   [OK] Selected day: " << day << endl;

    // Display session options based on credits
    displaySessionOptions(credits);

    // Input session choice with validation
    bool sessionValid = false;
    TimeSession selectedSession;

    do {
        sessionChoice = inputInteger("Select session time (enter number): ", 1, NUMBER_OF_SESSIONS);

        if (getSessionBasedOnChoice(credits, sessionChoice, selectedSession)) {
            sessionValid = true;
            cout << "   [OK] Selected session: " << selectedSession.startTime
                 << " - " << selectedSession.endTime << endl;
        } else {
            cout << "   [!] Session choice not valid for " << credits << " credits.\n" << endl;
        }
    } while (!sessionValid);

    cout << "Enter Room             : ";
    getline(cin, room);
    cout << "Enter Lecturer Name    : ";
    getline(cin, lecturer);

    // Find selected session index in SESSION_TIMES array
    int sessionIndex = -1;
    for (int i = 0; i < NUMBER_OF_SESSIONS; i++) {
        if (SESSION_TIMES[i].startTime == selectedSession.startTime &&
            SESSION_TIMES[i].endTime == selectedSession.endTime) {
            sessionIndex = i;
            break;
        }
    }

    ofstream file("courses.txt", ios::app);
    if (file.is_open()) {
        // Format: Name | StartTime | EndTime | Room | Credits | Lecturer | Day | SessionIndex
        file << courseName << " | " << selectedSession.startTime << " | " << selectedSession.endTime
             << " | " << room << " | " << credits << " | " << lecturer
             << " | " << day << " | " << sessionIndex << endl;

        cout << "\n>> Success! Course has been added.\n" << endl;
        file.close();
        loadCourseData();
    } else {
        cout << "\n>> Failed to write file.\n" << endl;
    }
}

void addNewStudent() {
    string studentName, studentID, courseInput, combinedCourses = "";

    printHeader("ADD NEW STUDENT DATA");
    cout << "\nStudent Name: ";
    getline(cin, studentName);
    cout << "Student ID  : ";
    getline(cin, studentID);

    // Find student for conflict check
    Student student;
    student.name = studentName;
    student.studentID = studentID;
    for (int k = 1; k <= MAX_SLOTS; k++) {
        student.schedule[k] = "Empty";
        student.room[k] = "-";
        student.startTime[k] = "-";
        student.endTime[k] = "-";
        student.day[k] = "-";
    }

    cout << "\nEnter courses taken (Type '0' when finished):\n" << endl;

    int count = 1;
    int totalCredits = 0;
    while(true) {
        cout << "Course " << count << ": ";
        getline(cin, courseInput);

        if (courseInput == "0") break;

        CourseMaster cm;
        if (!searchCourse(courseInput, cm)) {
            cout << "   [!] Warning: Course not found!\n" << endl;
        } else {
            // Check schedule conflict
            if (checkScheduleConflict(student, cm.day, cm.startTime, cm.endTime)) {
                cout << "   [!] WARNING: This course conflicts with already selected schedule!\n";
                cout << "   [!] Day: " << cm.day << ", Time: " << cm.startTime << "-" << cm.endTime << endl;
                cout << "   [!] Please select another course.\n" << endl;
                continue;
            }
            
            totalCredits += cm.credits;
            cout << "   [OK] " << cm.day << " | " << cm.startTime << "-" << cm.endTime
                 << " | " << cm.credits << " credits | " << cm.room
                 << " | Lecturer: " << cm.lecturer << "\n" << endl;
            
            // Update temporary student data
            for (int s = 1; s <= MAX_SLOTS; s++) {
                if (student.schedule[s] == "Empty") {
                    student.schedule[s] = courseInput;
                    student.room[s] = cm.room;
                    student.startTime[s] = cm.startTime;
                    student.endTime[s] = cm.endTime;
                    student.day[s] = cm.day;
                    break;
                }
            }
        }

        if (combinedCourses != "") combinedCourses += "|";
        combinedCourses += courseInput;
        count++;
        
        // Check if maximum slots reached
        if (count > MAX_SLOTS + 1) {
            cout << "   [!] Maximum " << MAX_SLOTS << " courses have been selected.\n" << endl;
            break;
        }
    }

    cout << "\nTotal credits taken: " << totalCredits << " credits\n" << endl;

    ofstream file("students.txt", ios::app);
    if (file.is_open()) {
        file << studentName << " | " << studentID << " | " << combinedCourses << endl;
        cout << ">> Success! Student data has been added.\n" << endl;
        file.close();
        loadStudentData();
    } else {
        cout << ">> Failed to write file.\n" << endl;
    }
}

// --- FUNCTIONS TO EDIT DATA ---
void editCourse() {
    if (numberOfCoursesRegistered == 0) {
        cout << "\n>> Course database is empty.\n" << endl;
        return;
    }

    printHeader("EDIT COURSE");

    cout << "\nCourse List:" << endl;
    printLine();
    for (int i = 0; i < numberOfCoursesRegistered; i++) {
        cout << (i+1) << ". " << courseDatabase[i].courseName
             << " (" << courseDatabase[i].day << " "
             << courseDatabase[i].startTime << "-"
             << courseDatabase[i].endTime << ")" << endl;
    }
    printLine();

    int choice = inputInteger("\nSelect course number to edit (0 to cancel): ", 0, numberOfCoursesRegistered);

    if (choice == 0) {
        cout << "\n>> Edit cancelled.\n" << endl;
        return;
    }

    int idx = choice - 1;
    string newName, newRoom, newLecturer, newDay;
    int newCredits, newSessionChoice;

    // Save old data for update
    string oldName = courseDatabase[idx].courseName;
    string oldDay = courseDatabase[idx].day;
    string oldStartTime = courseDatabase[idx].startTime;
    string oldEndTime = courseDatabase[idx].endTime;

    cout << "\nCurrent data:" << endl;
    cout << "Name   : " << courseDatabase[idx].courseName << endl;
    cout << "Day    : " << courseDatabase[idx].day << endl;
    cout << "Time   : " << courseDatabase[idx].startTime << "-" << courseDatabase[idx].endTime << endl;
    cout << "Room   : " << courseDatabase[idx].room << endl;
    cout << "Credits: " << courseDatabase[idx].credits << endl;
    cout << "Lecturer: " << courseDatabase[idx].lecturer << endl;

    cout << "\n(Press Enter to keep old value)\n" << endl;

    cout << "New Name     : ";
    getline(cin, newName);
    if (newName != "") courseDatabase[idx].courseName = newName;

    // Input new day
    cout << "New Day (type 'change' to change, or Enter to keep): ";
    string dayInput;
    getline(cin, dayInput);
    if (dayInput == "change") {
        newDay = selectDay();
        courseDatabase[idx].day = newDay;
        cout << "   [OK] Selected day: " << newDay << endl;
    }

    // If changing credits, show new sessions
    cout << "New Credits (enter 0 to not change): ";
    string creditsInput;
    getline(cin, creditsInput);

    if (creditsInput != "" && creditsInput != "0") {
        try {
            newCredits = stoi(creditsInput);
            if (newCredits >= 1 && newCredits <= 3) {
                courseDatabase[idx].credits = newCredits;

                // Show new session options based on new credits
                displaySessionOptions(newCredits);
                newSessionChoice = inputInteger("Select new session time: ", 1, NUMBER_OF_SESSIONS);

                TimeSession newSession;
                if (getSessionBasedOnChoice(newCredits, newSessionChoice, newSession)) {
                    courseDatabase[idx].startTime = newSession.startTime;
                    courseDatabase[idx].endTime = newSession.endTime;

                    // Update session index
                    for (int i = 0; i < NUMBER_OF_SESSIONS; i++) {
                        if (SESSION_TIMES[i].startTime == newSession.startTime &&
                            SESSION_TIMES[i].endTime == newSession.endTime) {
                            courseDatabase[idx].sessionIndex = i;
                            break;
                        }
                    }
                }
            }
        } catch (...) {
            cout << "   [!] Credits input invalid, using old value.\n" << endl;
        }
    }

    cout << "New Room     : ";
    getline(cin, newRoom);
    if (newRoom != "") courseDatabase[idx].room = newRoom;

    cout << "New Lecturer : ";
    getline(cin, newLecturer);
    if (newLecturer != "") courseDatabase[idx].lecturer = newLecturer;

    // Check conflict for students taking this course
    bool hasConflict = false;
    for (int i = 0; i < numberOfStudentsRegistered; i++) {
        for (int s = 1; s <= MAX_SLOTS; s++) {
            if (studentDatabase[i].schedule[s] == oldName) {
                Student tempStudent = studentDatabase[i];
                // Replace old course data with new in tempStudent
                for (int ts = 1; ts <= MAX_SLOTS; ts++) {
                    if (tempStudent.schedule[ts] == oldName) {
                        tempStudent.day[ts] = courseDatabase[idx].day;
                        tempStudent.startTime[ts] = courseDatabase[idx].startTime;
                        tempStudent.endTime[ts] = courseDatabase[idx].endTime;
                    }
                }
                
                // Check conflict with other courses
                if (checkScheduleConflict(tempStudent, courseDatabase[idx].day, 
                                         courseDatabase[idx].startTime, 
                                         courseDatabase[idx].endTime, 
                                         oldName)) {
                    hasConflict = true;
                    cout << "\n   [!] WARNING: This change will cause schedule conflict for student:\n";
                    cout << "   [!] " << studentDatabase[i].name << " (" << studentDatabase[i].studentID << ")\n";
                    cout << "   [!] Course: " << oldName << endl;
                }
                break;
            }
        }
    }

    if (hasConflict) {
        cout << "\nDo you still want to save changes? (y/n): ";
        char confirm;
        cin >> confirm;
        cin.ignore();
        
        if (confirm != 'y' && confirm != 'Y') {
            cout << "\n>> Changes cancelled.\n" << endl;
            // Restore old values
            courseDatabase[idx].courseName = oldName;
            courseDatabase[idx].day = oldDay;
            courseDatabase[idx].startTime = oldStartTime;
            courseDatabase[idx].endTime = oldEndTime;
            return;
        }
    }

    // Rewrite courses.txt file
    ofstream file("courses.txt");
    if (file.is_open()) {
        for (int i = 0; i < numberOfCoursesRegistered; i++) {
            file << courseDatabase[i].courseName << " | "
                 << courseDatabase[i].startTime << " | "
                 << courseDatabase[i].endTime << " | "
                 << courseDatabase[i].room << " | "
                 << courseDatabase[i].credits << " | "
                 << courseDatabase[i].lecturer << " | "
                 << courseDatabase[i].day << " | "
                 << courseDatabase[i].sessionIndex << endl;
        }
        file.close();

        // IMPORTANT UPDATE: Also update data in students.txt if changed
        if (newName != "" && newName != oldName) {
            // Update in array database first
            for (int i = 0; i < numberOfStudentsRegistered; i++) {
                for (int s = 1; s <= MAX_SLOTS; s++) {
                    if (studentDatabase[i].schedule[s] == oldName) {
                        studentDatabase[i].schedule[s] = newName;
                    }
                }
            }
        }

        // Update day, time, and room in student schedules
        for (int i = 0; i < numberOfStudentsRegistered; i++) {
            for (int s = 1; s <= MAX_SLOTS; s++) {
                if (studentDatabase[i].schedule[s] == courseDatabase[idx].courseName) {
                    studentDatabase[i].room[s] = courseDatabase[idx].room;
                    studentDatabase[i].startTime[s] = courseDatabase[idx].startTime;
                    studentDatabase[i].endTime[s] = courseDatabase[idx].endTime;
                    studentDatabase[i].day[s] = courseDatabase[idx].day;
                }
            }
        }

        // Rewrite students.txt with updated data
        ofstream studentFile("students.txt");
        if (studentFile.is_open()) {
            for (int i = 0; i < numberOfStudentsRegistered; i++) {
                studentFile << studentDatabase[i].name << " | " << studentDatabase[i].studentID << " | ";

                bool first = true;
                for (int s = 1; s <= MAX_SLOTS; s++) {
                    if (studentDatabase[i].schedule[s] != "Empty" && studentDatabase[i].schedule[s] != "-") {
                        if (!first) studentFile << "|";
                        studentFile << studentDatabase[i].schedule[s];
                        first = false;
                    }
                }
                studentFile << endl;
            }
            studentFile.close();
        }

        cout << "\n>> Data successfully updated!\n" << endl;

        // Reload data
        loadCourseData();
        loadStudentData();

    } else {
        cout << "\n>> Failed to write file.\n" << endl;
    }
}

void editStudent() {
    if (numberOfStudentsRegistered == 0) {
        cout << "\n>> Student database is empty.\n" << endl;
        return;
    }

    printHeader("EDIT STUDENT DATA");

    cout << "\nStudent List:" << endl;
    printLine();
    for (int i = 0; i < numberOfStudentsRegistered; i++) {
        cout << (i+1) << ". " << studentDatabase[i].name
             << " (" << studentDatabase[i].studentID << ")" << endl;
    }
    printLine();

    int choice = inputInteger("\nSelect student number to edit (0 to cancel): ", 0, numberOfStudentsRegistered);

    if (choice == 0) {
        cout << "\n>> Edit cancelled.\n" << endl;
        return;
    }

    int idx = choice - 1;
    string newName, newStudentID;

    cout << "\nCurrent data:" << endl;
    cout << "Name: " << studentDatabase[idx].name << endl;
    cout << "ID  : " << studentDatabase[idx].studentID << endl;

    cout << "\n(Press Enter to keep old value)\n" << endl;

    cout << "New Name: ";
    getline(cin, newName);
    if (newName != "") studentDatabase[idx].name = newName;

    cout << "New ID   : ";
    getline(cin, newStudentID);
    if (newStudentID != "") studentDatabase[idx].studentID = newStudentID;

    // Rewrite file
    ofstream file("students.txt");
    if (file.is_open()) {
        for (int i = 0; i < numberOfStudentsRegistered; i++) {
            file << studentDatabase[i].name << " | " << studentDatabase[i].studentID << " | ";

            bool first = true;
            for (int s = 1; s <= MAX_SLOTS; s++) {
                if (studentDatabase[i].schedule[s] != "Empty" && studentDatabase[i].schedule[s] != "-") {
                    if (!first) file << "|";
                    file << studentDatabase[i].schedule[s];
                    first = false;
                }
            }
            file << endl;
        }
        file.close();
        cout << "\n>> Data successfully updated!\n" << endl;
    } else {
        cout << "\n>> Failed to write file.\n" << endl;
    }
}

// --- FUNCTIONS TO DELETE DATA ---
void deleteCourse() {
    if (numberOfCoursesRegistered == 0) {
        cout << "\n>> Course database is empty.\n" << endl;
        return;
    }

    printHeader("DELETE COURSE");

    cout << "\nCourse List:" << endl;
    printLine();
    for (int i = 0; i < numberOfCoursesRegistered; i++) {
        cout << (i+1) << ". " << courseDatabase[i].courseName
             << " (" << courseDatabase[i].day << " "
             << courseDatabase[i].startTime << "-"
             << courseDatabase[i].endTime << ")" << endl;
    }
    printLine();

    int choice = inputInteger("\nSelect course number to delete (0 to cancel): ", 0, numberOfCoursesRegistered);

    if (choice == 0) {
        cout << "\n>> Delete cancelled.\n" << endl;
        return;
    }

    int idx = choice - 1;
    string courseName = courseDatabase[idx].courseName;

    cout << "\nAre you sure you want to delete course '" << courseName << "'? (y/n): ";
    char confirm;
    cin >> confirm;
    cin.ignore();

    if (confirm != 'y' && confirm != 'Y') {
        cout << "\n>> Delete cancelled.\n" << endl;
        return;
    }

    // Remove from array (shift elements)
    for (int i = idx; i < numberOfCoursesRegistered - 1; i++) {
        courseDatabase[i] = courseDatabase[i + 1];
    }
    numberOfCoursesRegistered--;

    // Rewrite file
    ofstream file("courses.txt");
    if (file.is_open()) {
        for (int i = 0; i < numberOfCoursesRegistered; i++) {
            file << courseDatabase[i].courseName << " | "
                 << courseDatabase[i].startTime << " | "
                 << courseDatabase[i].endTime << " | "
                 << courseDatabase[i].room << " | "
                 << courseDatabase[i].credits << " | "
                 << courseDatabase[i].lecturer << " | "
                 << courseDatabase[i].day << " | "
                 << courseDatabase[i].sessionIndex << endl;
        }
        file.close();

        // Remove course from student schedules
        for (int i = 0; i < numberOfStudentsRegistered; i++) {
            for (int s = 1; s <= MAX_SLOTS; s++) {
                if (studentDatabase[i].schedule[s] == courseName) {
                    studentDatabase[i].schedule[s] = "Empty";
                    studentDatabase[i].room[s] = "-";
                    studentDatabase[i].startTime[s] = "-";
                    studentDatabase[i].endTime[s] = "-";
                    studentDatabase[i].day[s] = "-";
                }
            }
        }

        // Update student file
        ofstream studentFile("students.txt");
        if (studentFile.is_open()) {
            for (int i = 0; i < numberOfStudentsRegistered; i++) {
                studentFile << studentDatabase[i].name << " | " << studentDatabase[i].studentID << " | ";

                bool first = true;
                for (int s = 1; s <= MAX_SLOTS; s++) {
                    if (studentDatabase[i].schedule[s] != "Empty" && studentDatabase[i].schedule[s] != "-") {
                        if (!first) studentFile << "|";
                        studentFile << studentDatabase[i].schedule[s];
                        first = false;
                    }
                }
                studentFile << endl;
            }
            studentFile.close();
        }

        cout << "\n>> Course '" << courseName << "' successfully deleted!\n" << endl;
    } else {
        cout << "\n>> Failed to write file.\n" << endl;
    }
}

void deleteStudent() {
    if (numberOfStudentsRegistered == 0) {
        cout << "\n>> Student database is empty.\n" << endl;
        return;
    }

    printHeader("DELETE STUDENT DATA");

    cout << "\nStudent List:" << endl;
    printLine();
    for (int i = 0; i < numberOfStudentsRegistered; i++) {
        cout << (i+1) << ". " << studentDatabase[i].name
             << " (" << studentDatabase[i].studentID << ")" << endl;
    }
    printLine();

    int choice = inputInteger("\nSelect student number to delete (0 to cancel): ", 0, numberOfStudentsRegistered);

    if (choice == 0) {
        cout << "\n>> Delete cancelled.\n" << endl;
        return;
    }

    int idx = choice - 1;
    string studentName = studentDatabase[idx].name;

    cout << "\nAre you sure you want to delete student '" << studentName << "'? (y/n): ";
    char confirm;
    cin >> confirm;
    cin.ignore();

    if (confirm != 'y' && confirm != 'Y') {
        cout << "\n>> Delete cancelled.\n" << endl;
        return;
    }

    // Remove from array (shift elements)
    for (int i = idx; i < numberOfStudentsRegistered - 1; i++) {
        studentDatabase[i] = studentDatabase[i + 1];
    }
    numberOfStudentsRegistered--;

    // Rewrite file
    ofstream file("students.txt");
    if (file.is_open()) {
        for (int i = 0; i < numberOfStudentsRegistered; i++) {
            file << studentDatabase[i].name << " | " << studentDatabase[i].studentID << " | ";

            bool first = true;
            for (int s = 1; s <= MAX_SLOTS; s++) {
                if (studentDatabase[i].schedule[s] != "Empty" && studentDatabase[i].schedule[s] != "-") {
                    if (!first) file << "|";
                    file << studentDatabase[i].schedule[s];
                    first = false;
                }
            }
            file << endl;
        }
        file.close();
        cout << "\n>> Student '" << studentName << "' successfully deleted!\n" << endl;
    } else {
        cout << "\n>> Failed to write file.\n" << endl;
    }
}

// --- OUTPUT ---
void displayAllCourses() {
    printHeader("COURSE DATABASE");

    if (numberOfCoursesRegistered == 0) {
        cout << "\n(Database is empty)\n" << endl;
        return;
    }

    cout << "\n" << left << setw(4) << "No"
         << setw(30) << "Course"
         << setw(8) << "Credits"
         << setw(12) << "Day"
         << setw(20) << "Time"
         << setw(10) << "Room"
         << "Lecturer" << endl;
    printLine();

    for (int i = 0; i < numberOfCoursesRegistered; i++) {
        string time = courseDatabase[i].startTime + "-" + courseDatabase[i].endTime;
        cout << left << setw(4) << (i + 1)
             << setw(30) << courseDatabase[i].courseName
             << setw(8) << courseDatabase[i].credits
             << setw(12) << courseDatabase[i].day
             << setw(20) << time
             << setw(10) << courseDatabase[i].room
             << courseDatabase[i].lecturer << endl;
    }
    printLine();
    cout << "\nTotal: " << numberOfCoursesRegistered << " courses registered.\n" << endl;
}

void displaySchedule(string searchName) {
    bool found = false;
    for (int i = 0; i < numberOfStudentsRegistered; i++) {
        if (studentDatabase[i].name == searchName) {
            found = true;

            int headerLength = max(70, (int)searchName.length() + 30);
            cout << "\n" << string(headerLength, '=') << endl;
            cout << "  CLASS SCHEDULE" << endl;
            cout << string(headerLength, '=') << endl;
            cout << "\nName : " << studentDatabase[i].name << endl;
            cout << "ID   : " << studentDatabase[i].studentID << "\n" << endl;

            cout << left << setw(6) << "No"
                 << setw(12) << "Day"
                 << setw(20) << "Time"
                 << setw(30) << "Course"
                 << setw(10) << "Room"
                 << "Status" << endl;
            printLineDynamic(88);

            int totalClasses = 0, totalCredits = 0;
            int no = 1;
            
            // Sort by day and time
            vector<pair<int, string>> sortedSchedule; // slot index, description
            
            for (int s = 1; s <= MAX_SLOTS; s++) {
                if (studentDatabase[i].schedule[s] != "Empty" && studentDatabase[i].schedule[s] != "-") {
                    string description = studentDatabase[i].day[s] + "|" + 
                                       studentDatabase[i].startTime[s] + "|" + 
                                       to_string(s);
                    sortedSchedule.push_back({s, description});
                }
            }
            
            // Sort by day then time
            sort(sortedSchedule.begin(), sortedSchedule.end(), [](const pair<int, string>& a, const pair<int, string>& b) {
                // Sort by day
                string dayA = a.second.substr(0, a.second.find('|'));
                string dayB = b.second.substr(0, b.second.find('|'));
                
                // Find day index
                int idxA = -1, idxB = -1;
                for (int h = 0; h < NUMBER_OF_DAYS; h++) {
                    if (DAYS[h] == dayA) idxA = h;
                    if (DAYS[h] == dayB) idxB = h;
                }
                
                if (idxA != idxB) return idxA < idxB;
                
                // If same day, sort by time
                string timeA = a.second.substr(dayA.length() + 1);
                timeA = timeA.substr(0, timeA.find('|'));
                string timeB = b.second.substr(dayB.length() + 1);
                timeB = timeB.substr(0, timeB.find('|'));
                
                return timeA < timeB;
            });
            
            for (const auto& item : sortedSchedule) {
                int s = item.first;
                totalClasses++;
                string time = studentDatabase[i].startTime[s] + "-" + studentDatabase[i].endTime[s];

                CourseMaster cm;
                if (searchCourse(studentDatabase[i].schedule[s], cm)) {
                    totalCredits += cm.credits;
                }

                cout << left << setw(6) << no++
                     << setw(12) << studentDatabase[i].day[s]
                     << setw(20) << time
                     << setw(30) << studentDatabase[i].schedule[s]
                     << setw(10) << studentDatabase[i].room[s]
                     << "Busy" << endl;
            }

            if (totalClasses == 0) {
                cout << "        (No schedule registered)" << endl;
            }

            printLineDynamic(88);
            cout << "\nTotal Classes: " << totalClasses << " | Total Credits: " << totalCredits << "\n" << endl;
            break;
        }
    }
    if (!found) {
        cout << "\n>> Student not found in database.\n" << endl;
    }
}

void checkAvailability(string day, string time) {
    int headerLength = max(60, (int)(day + " " + time).length() + 40);
    cout << "\n" << string(headerLength, '=') << endl;
    cout << "  CHECK TIME AVAILABILITY" << endl;
    cout << string(headerLength, '=') << endl;
    cout << "\nDay  : " << day << endl;
    cout << "Time : " << time << "\n" << endl;

    int busy = 0, available = 0;

    cout << "NOT AVAILABLE (In Class):" << endl;
    printLine();
    for (int i = 0; i < numberOfStudentsRegistered; i++) {
        for (int s = 1; s <= MAX_SLOTS; s++) {
            if (studentDatabase[i].day[s] == day && studentDatabase[i].startTime[s] == time) {
                busy++;
                cout << busy << ". " << studentDatabase[i].name
                     << " (" << studentDatabase[i].studentID << ")" << endl;
                cout << "   Course : " << studentDatabase[i].schedule[s]
                     << " | Room: " << studentDatabase[i].room[s] << endl;
                break;
            }
        }
    }
    if (busy == 0) cout << "(No students are busy)\n" << endl;

    cout << "\nAVAILABLE (Free Time):" << endl;
    printLine();
    for (int i = 0; i < numberOfStudentsRegistered; i++) {
        bool isAvailable = true;
        for (int s = 1; s <= MAX_SLOTS; s++) {
            if (studentDatabase[i].day[s] == day && studentDatabase[i].startTime[s] == time) {
                isAvailable = false;
                break;
            }
        }
        if (isAvailable) {
            available++;
            cout << available << ". " << studentDatabase[i].name
                 << " (" << studentDatabase[i].studentID << ")" << endl;
        }
    }
    if (available == 0) cout << "(All students are busy)\n" << endl;

    printLine();
    cout << "\nSummary: " << busy << " Busy | " << available << " Available\n" << endl;
}

void displayAllSchedules() {
    if (numberOfStudentsRegistered == 0) {
        cout << "\n(No student data yet)\n" << endl;
        return;
    }

    printHeader("ALL STUDENTS LIST");
    cout << "\n" << left << setw(4) << "No"
         << setw(25) << "Name"
         << setw(15) << "Student ID"
         << "Total Classes" << endl;
    printLine();

    for (int i = 0; i < numberOfStudentsRegistered; i++) {
        int totalClasses = 0;
        for (int s = 1; s <= MAX_SLOTS; s++) {
            if (studentDatabase[i].schedule[s] != "Empty" && studentDatabase[i].schedule[s] != "-") {
                totalClasses++;
            }
        }

        cout << left << setw(4) << (i+1)
             << setw(25) << studentDatabase[i].name
             << setw(15) << studentDatabase[i].studentID
             << totalClasses << " classes" << endl;
    }
    printLine();
    cout << "\nTotal: " << numberOfStudentsRegistered << " students registered.\n" << endl;
}

void exportSchedule() {
    if (numberOfStudentsRegistered == 0) {
        cout << "\n>> No data to export.\n" << endl;
        return;
    }

    ofstream file("export_schedule.txt");
    if (!file.is_open()) {
        cout << "\n>> Failed to create export file.\n" << endl;
        return;
    }

    file << "============================================================\n";
    file << "         STUDENT CLASS SCHEDULE REPORT\n";
    file << "============================================================\n\n";

    for (int i = 0; i < numberOfStudentsRegistered; i++) {
        file << "Student: " << studentDatabase[i].name
             << " (" << studentDatabase[i].studentID << ")\n";
        file << "------------------------------------------------------------\n";

        // Sort by day and time
        vector<pair<int, string>> sortedSchedule; // slot index, description
        
        for (int s = 1; s <= MAX_SLOTS; s++) {
            if (studentDatabase[i].schedule[s] != "Empty" && studentDatabase[i].schedule[s] != "-") {
                string description = studentDatabase[i].day[s] + "|" + 
                                   studentDatabase[i].startTime[s] + "|" + 
                                   to_string(s);
                sortedSchedule.push_back({s, description});
            }
        }
        
        // Sort by day then time
        sort(sortedSchedule.begin(), sortedSchedule.end(), [](const pair<int, string>& a, const pair<int, string>& b) {
            string dayA = a.second.substr(0, a.second.find('|'));
            string dayB = b.second.substr(0, b.second.find('|'));
            
            int idxA = -1, idxB = -1;
            for (int h = 0; h < NUMBER_OF_DAYS; h++) {
                if (DAYS[h] == dayA) idxA = h;
                if (DAYS[h] == dayB) idxB = h;
            }
            
            if (idxA != idxB) return idxA < idxB;
            
            string timeA = a.second.substr(dayA.length() + 1);
            timeA = timeA.substr(0, timeA.find('|'));
            string timeB = b.second.substr(dayB.length() + 1);
            timeB = timeB.substr(0, timeB.find('|'));
            
            return timeA < timeB;
        });
        
        int totalClasses = 0;
        for (const auto& item : sortedSchedule) {
            int s = item.first;
            totalClasses++;
            string time = studentDatabase[i].startTime[s] + "-" + studentDatabase[i].endTime[s];
            file << "Day  : " << studentDatabase[i].day[s] << "\n";
            file << "Time : " << time << "\n";
            file << "  Course : " << studentDatabase[i].schedule[s] << "\n";
            file << "  Room   : " << studentDatabase[i].room[s] << "\n";

            CourseMaster cm;
            if (searchCourse(studentDatabase[i].schedule[s], cm)) {
                file << "  Lecturer: " << cm.lecturer << "\n";
                file << "  Credits : " << cm.credits << "\n\n";
            }
        }
        file << "Total: " << totalClasses << " classes\n";
        file << "============================================================\n\n";
    }

    file.close();
    cout << "\n>> Schedule successfully exported to 'export_schedule.txt'\n" << endl;
}

// Function to clear terminal
void clearTerminal() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

int main() {
    // Create empty files if not exist
    ofstream f1("courses.txt", ios::app); f1.close();
    ofstream f2("students.txt", ios::app); f2.close();

    // Load data from files
    loadCourseData();
    loadStudentData();

    int choice;
    do {
        printHeader("CLASS SCHEDULING SYSTEM");
        cout << "\nMAIN MENU:\n" << endl;
        cout << "  1. Input New Data" << endl;
        cout << "  2. View Student Schedule (Individual)" << endl;
        cout << "  3. View All Registered Students" << endl;
        cout << "  4. Check Time Availability (Who is free)" << endl;
        cout << "  5. Edit Data" << endl;
        cout << "  6. Delete Data" << endl;
        cout << "  7. View Course Database" << endl;
        cout << "  8. Export Schedule to File" << endl;
        cout << "  9. Clear Terminal" << endl;
        cout << "  0. Exit" << endl;

        cout << "\n=== Current Database Status ===" << endl;
        cout << "Data successfully loaded:" << endl;
        cout << "- Courses registered: " << numberOfCoursesRegistered << endl;
        cout << "- Students registered: " << numberOfStudentsRegistered << endl;

        cout << "\n------------------------------------------------------------" << endl;
        cout << "Your choice: ";

        // Main menu input validation
        while (!(cin >> choice)) {
            cout << "\n   [!] Input must be a number! Please try again.\n" << endl;
            cout << "Your choice: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        cin.ignore();

        switch(choice) {
            case 1: {
                char subChoice;
                do{
                cout << "\n   a. Add Course" << endl;
                cout << "   b. Add Student Data" << endl;
                cout << "\n   Choice (a/b) (type 0 when done): ";
                cin >> subChoice;
                cin.ignore();

                if (subChoice == 'a' || subChoice == 'A') {
                    addNewCourse();
                } else if (subChoice == 'b' || subChoice == 'B') {
                    displayAllCourses();
                    addNewStudent();}
                else if (subChoice =='0'){
                    break;
                }
                else{
                    cout << "   Input must be letter a/b!!" <<endl;
                }
                }while(subChoice != 'a' || subChoice != 'A' || subChoice != 'b' || subChoice != 'B');
                break;
            }
            case 2: {
                string name;
                cout << "\nEnter Student Name: ";
                getline(cin, name);
                displaySchedule(name);
                break;
            }
            case 3: {
                displayAllSchedules();
                break;
            }
            case 4: {
                string day, time;
                
                // Select day
                displayDayOptions();
                int dayChoice = inputInteger("Select day (1-5): ", 1, NUMBER_OF_DAYS);
                day = DAYS[dayChoice - 1];
                
                cout << "\nEnter Start Time (HH.MM, example: 07.30): ";
                getline(cin, time);

                // Validate time format
                if (time.length() == 5 && time[2] == '.') {
                    checkAvailability(day, time);
                } else {
                    cout << "\n>> Time format invalid!\n" << endl;
                }
                break;
            }
            case 5: {
                cout << "\n   a. Edit Course" << endl;
                cout << "   b. Edit Student Data" << endl;
                cout << "\n   Choice (a/b): ";
                char subChoice;
                cin >> subChoice;
                cin.ignore();

                if (subChoice == 'a' || subChoice == 'A') {
                    editCourse();
                } else if (subChoice == 'b' || subChoice == 'B') {
                    editStudent();
                }
                break;
            }
            case 6: {
                cout << "\n   a. Delete Course" << endl;
                cout << "   b. Delete Student Data" << endl;
                cout << "\n   Choice (a/b): ";
                char subChoice;
                cin >> subChoice;
                cin.ignore();

                if (subChoice == 'a' || subChoice == 'A') {
                    deleteCourse();
                } else if (subChoice == 'b' || subChoice == 'B') {
                    deleteStudent();
                }
                break;
            }
            case 7: {
                displayAllCourses();
                break;
            }
            case 8: {
                exportSchedule();
                break;
            }
            case 9: {
                clearTerminal();
                cout << "Terminal has been cleared.\n" << endl;
                continue;
            }
            case 0: {
                printHeader("THANK YOU");
                cout << "\nProgram finished. Thank you very much for using this program :)\n" << endl;
                break;
            }
            default: {
                cout << "\n>> Invalid choice. Please try again..\n" << endl;
            }
        }

        if (choice != 0 && choice != 10) {
            cout << "\nPress Enter to continue...";
            cin.get();
        }

    } while (choice != 0);

    return 0;
}