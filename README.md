🚘 VEHICLE RENTAL SYSTEM
📌 Curated by UMAIR IMRAN

📝 Project Overview
The Vehicle Rental System is a vibrant and user-friendly Command Line Interface (CLI) application designed to simulate a real-world vehicle rental business. Built in C++, this system allows users to view, rent, and return different types of vehicles — with a colorful and emoji-enhanced interface that improves user experience.

🌟 Key Features
🔹 1. View Vehicles
Users can view all available vehicle types such as Cars 🚗, Bikes 🏍️, SUVs 🚙, Buses 🚌, Trucks 🚛, Vans 🚐, and Rickshaws 🛺.

Each type includes seven unique models, each with:

A model name

A relevant emoji/sticker 🐯, 🦄, 🐘 etc.

A color code for visual identity

A daily rent rate (e.g., Rs. 5000/day)

🔹 2. Rent a Vehicle
Users can choose a vehicle type and select a specific model.

They’re prompted to enter the number of days they wish to rent.

A "Point to Ponder" 🧠 warning appears:

🧠 Point to Ponder: If you break any part of the vehicle, a Rs. 2200 challan will be applied!

Final rent is calculated and displayed with color highlights.

Helps simulate real-world decision-making in rentals.

🔹 3. Return Vehicle
When returning the vehicle, the system asks if it was broken/damaged.

If "Yes", an additional challan (fine) is applied:

Rs. 2200 for cars 🚗

Custom challans (e.g., Rs. 1800 for bikes 🏍️, Rs. 2500 for buses 🚌)

Final cost is calculated with the penalty and shown to the user.

🔹 4. Colorful & Emoji-Based Output
Uses ANSI escape codes for colored text (e.g., \033[31m for red).

Models and vehicle types include cute and relevant emojis.

Adds visual appeal and increases usability — ideal for students and beginners.

🧠 Point to Ponder System
A proactive warning system that simulates real-life vehicle care.

Highlights the importance of responsibility during rentals.

Encourages ethical return behavior and adds realism to the simulation.

🏗️ System Architecture
Uses struct to define:

Model: Holds model info, emoji, color, and rent

VehicleType: Holds vehicle type name, emoji, color, and its list of models

Stores all data in a vector<VehicleType> called vehicleDB

Main menu is loop-based using a while(true) construct

💻 Technologies Used
Language: C++

Concepts: Structs, Vectors, Input/Output, Conditional Logic

UI Enhancements: ANSI Color Codes, Emojis, Formatted Output (iomanip)

✅ Why This System is Unique?
Not just a dry C++ console program — it’s interactive, colorful, and expressive

Designed to look human-made, not AI-generated

Suitable for:

C++ assignments

Fun projects

Teaching tools for object-oriented and structured programming

📌 Sample Console Output
markdown
Copy
Edit
🚘 VEHICLE RENTAL SYSTEM
📌 Curated by UMAIR IMRAN

🔸 MAIN MENU
1. View Vehicles
2. Rent a Vehicle
3. Return a Vehicle
4. Exit

➡️  Enter your choice: 2

🌟 Available Vehicle Types:
1. 🚗 Car
2. 🏍️ Bike
...

🧠 Point to Ponder: If you break any part of the vehicle, a Rs. 2200 challan will be applied!
...
✅ Total Rent: Rs. 17200
🧾 Future Enhancements (Optional)
Admin panel with password for adding new vehicles

Persistent file handling to store user rentals

Receipt generation as a downloadable .txt file

User login/registration system

Dynamic pricing (based on seasons or availability)

