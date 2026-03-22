<div align="center">
  <h1>✨ Draftify</h1>
  <p><em>The Ultimate Smart Content Creation Studio Desktop Application</em></p>

  <p>
    <img src="https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white" alt="C++" />
    <img src="https://img.shields.io/badge/Qt-%23217346.svg?style=for-the-badge&logo=Qt&logoColor=white" alt="Qt" />
    <img src="https://img.shields.io/badge/Oracle-F80000?style=for-the-badge&logo=oracle&logoColor=white" alt="Oracle" />
    <img src="https://img.shields.io/badge/Arduino-00979D?style=for-the-badge&logo=Arduino&logoColor=white" alt="Arduino" />
    <img src="https://img.shields.io/badge/Eco--Friendly-Paperless-4CAF50?style=for-the-badge&logo=leaf&logoColor=white" alt="Eco-Friendly" />
  </p>
</div>

---

## 📖 Overview

**Draftify** is a powerful **Smart Content Creation Studio** desktop application built entirely in C++ with the **Qt Framework**. It offers a seamless, all-in-one centralized platform designed specifically for digital agencies and studios to manage their employees, content creators (clients), projects, and digital platforms.

Using modern UI/UX paradigms, Draftify features beautiful animated charts, calendar integration, smart project tracking, and seamlessly communicates with standard hardware interfaces (like **Arduino via Serial Port**). 

🌱 **An Eco-Friendly & Paperless Initiative:** Draftify is built with a deep respect for the environment. By digitizing employee records, creator contracts, project workflows, and physical ratings, we eliminate the need for paper-based administration. This drastically reduces the studio's carbon footprint and ensures a 100% digital, optimized workflow.

---

## 🌟 Strong Points & Core Features

### 🏢 Employee & Human Resources Management
- **Intelligent Role-Based Access:** Not everyone needs everything! Draftify utilizes a smart login system where permissions are governed by roles. Managers, HRs, and standard staff only see the features and data they have access to, securing the studio's internal data.
- **Full Employee CRUD**: Maintain exhaustive digital records of your staff.
- **Advanced Authentication**: Secure login, signup flows, password recovery with custom security questions.
- **Reporting & Statistics**: Visualize salaries per role dynamically via Qt Charts, completely bypassing the need for printed spreadsheets.
- **PDF Export**: Generate official employee reports natively. 

### 👥 Content Creator (Client) Management
- **Interactive Profile Cards:** Say goodbye to boring tables! Hover over a content creator in the app to reveal **beautiful, animated profile cards** displaying their photo, followers, and active platforms instantly.
- **The Creator Podium:** A visually stunning podium interface highlights your top-performing creators (based on follower count or engagement), making success visually rewarding.
- **Creator Database (Full CRUD)**: Full database management of content creators and signed influencers.
- **Followers Tracking**: Manage, track, and sort your creators by their followers count in real-time.

### 📊 Platform Management 
- **Social Media Connectivity**: Check interactions across platforms (Instagram, TikTok, Twitter, Facebook).
- **Intelligent Statistics**: Compare platform popularity through dynamically generated pie and bar charts.

### 🚀 Smart Project Management
- **Project Tracking**: Lifecycle management of digital projects.
- **Calendar Integration**: Interactive, color-coded task assignments based on status and dynamically calculated deadlines.
- **Smart Checklists**: Automatic, customizable task checklists based on the project category.

### 🔌 Hardware Integration (Smart Arduino Systems)
- **Arduino RFID Smart Badges**: Physical security meets digital access! Employees or Creators simply scan their smart RFID badges over the Arduino hardware to instantly log into their profiles or verify their attendance, streamlining access and avoiding manual typing.
- **Hardware Feedback & Rating Module**: An interactive physical rating terminal (powered by Arduino) allows live audiences, staff, or guests to leave physical ratings. The Arduino captures this data and transmits it instantaneously via the Serial Port directly into Draftify's Oracle Database for immediate chart generation.

---

## 🛠️ Technology Stack

| Component               | Technology / Library          |
|-------------------------|-------------------------------|
| **Primary Language**    | C++ 17                        |
| **UI Framework**        | Qt 6 (Widgets, PrintSupport)  |
| **Data Visualization**  | Qt Charts                     |
| **Database**            | Oracle SQL (via QODBC)        |
| **Hardware Comms**      | Qt SerialPort                 |

---

## ⚡ Setup & Installation

### Prerequisites
1. **Qt Creator** installed (Version 5 or 6+)
2. **Oracle Database** with a configured ODBC connection named `draftify`
3. Optional: **Arduino Board** connected to an available COM port for RFID/Rating functionalities.

### Running the App
1. Clone / Download this repository.
2. Open `Draftify.pro` in **Qt Creator**.
3. Create an ODBC data source matching these credentials:
   - **DSN**: `draftify`
   - **User**: `optime`
   - **Password**: `123`
4. Build and Run (`Ctrl+R`).

---

## 🎨 UI & Aesthetics

Draftify isn't just about managing data; it's about doing it **beautifully**. We've ditched the standard boring desktop application conventions:
- **Hover Effects & Interactive Elements**: Elements like the client profile cards scale, show shadows, and reveal hidden information simply by hovering.
- **Responsive Layouts**: Screens elegantly adapt to manage complex datasets.
- **Micro-Animations**: Experience smooth transitions, progressive bar chart loadings (with custom animations), and immediate UI feedback.
- **Rich Iconography**: Hand-picked Qt Resource (`.qrc`) assets keep the interface lively and intuitive.

<br>
<div align="center">
  <i>Engineered for enterprise performance and reliability using C++ & the Qt Framework.</i>
</div>
