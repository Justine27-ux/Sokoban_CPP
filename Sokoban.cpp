// =====================================================
//  GAME SOKOBAN (versi ncurses) - 5 LEVEL
//  Cara compile : g++ main.cpp -o main -lncurses
//  Cara jalan   : ./main
//  Tujuan game  : dorong semua kotak (B) ke titik target (.)
// =====================================================

#include <ncurses.h>   // library untuk membuat tampilan game di terminal

using namespace std;

// Representasi Peta (Map):
// '#' = Dinding
// ' ' = Lantai kosong
// 'P' = Pemain (Player)
// 'B' = Kotak (Box)
// '.' = Target penyimpanan kotak
// '+' = Pemain di atas target
// '*' = Kotak di atas target (Goal)

// Ukuran peta (semua level memakai ukuran yang sama)
const int MAP_HEIGHT = 14;   // jumlah baris
const int MAP_WIDTH = 30;    // jumlah kolom
const int TOTAL_LEVEL = 5;   // jumlah level di dalam game

// Nama tiap level, ditampilkan di atas peta
const char* levelNames[TOTAL_LEVEL] = {
    "Pemanasan",
    "Dua Ruangan",
    "Pintu Sempit",
    "Gudang Buntu",
    "Labirin Gudang"
};

// Data semua level. Level makin ke bawah makin susah.
// Indeks pertama = nomor level (0 sampai 4)
char allLevels[TOTAL_LEVEL][MAP_HEIGHT][MAP_WIDTH + 1] = {

    // ----- LEVEL 1 : 2 kotak, ruangan terbuka -----
    {
        "##############################",
        "#                            #",
        "#                            #",
        "#   P                        #",
        "#          B                 #",
        "#             ###            #",
        "#             ###     .      #",
        "#                            #",
        "#          B                 #",
        "#                            #",
        "#                     .      #",
        "#                            #",
        "#                            #",
        "##############################"
    },

    // ----- LEVEL 2 : 3 kotak, ada sekat dengan celah -----
    {
        "##############################",
        "#           #                #",
        "#   P       #                #",
        "#     B     #         .      #",
        "#           #                #",
        "#                            #",
        "#       B                    #",
        "#                        .   #",
        "#           #                #",
        "#    B      #                #",
        "#           #    ###   .     #",
        "#           #    ###         #",
        "#           #                #",
        "##############################"
    },

    // ----- LEVEL 3 : 4 kotak, harus lewat pintu sempit -----
    {
        "##############################",
        "#         #                  #",
        "#  P      #                  #",
        "#    B    #    ##        .   #",
        "#         #    ##            #",
        "#      B  #           .      #",
        "#                            #",
        "#         #        ###########",
        "#   B     #                  #",
        "#         #             .    #",
        "#      B  #                  #",
        "#         #               .  #",
        "#         #                  #",
        "##############################"
    },

    // ----- LEVEL 4 : 4 kotak, target ada di lorong buntu -----
    // Tips: isi target yang paling ujung (kanan) dulu!
    {
        "##############################",
        "#                  ###########",
        "#    B             ###########",
        "#       ###        ###########",
        "#       ###  B     ###########",
        "#            #     ###########",
        "#  P                     ....#",
        "#                  ###########",
        "#       ###        ###########",
        "#     B ###        ###########",
        "#             B    ###########",
        "#                  ###########",
        "#                  ###########",
        "##############################"
    },

    // ----- LEVEL 5 : 5 kotak, dua lorong buntu, banyak penghalang -----
    {
        "##############################",
        "#          ###################",
        "#     B    ###################",
        "#   ##                    ...#",
        "#   ##     ###################",
        "#        B ###################",
        "# P    ##  ###################",
        "#      ##B ###################",
        "#     B    ###################",
        "#  ##                      ..#",
        "#  ##   ## ###################",
        "#    B  ## ###################",
        "#          ###################",
        "##############################"
    }
};

// Variabel global (bisa dipakai oleh semua fungsi)
char currentMap[MAP_HEIGHT][MAP_WIDTH + 1];   // peta yang sedang dimainkan
int playerX, playerY;      // posisi pemain
int totalBoxes = 0;        // jumlah kotak di level ini
int currentLevel = 0;      // level sekarang (0 = level 1)
int moveCount = 0;         // jumlah langkah di level ini
int totalMoves = 0;        // jumlah langkah dari semua level

// Fungsi untuk memuat (atau mengulang) sebuah level
// Peta dari allLevels disalin ke currentMap, lalu posisi pemain dicari
void loadLevel(int level) {
    totalBoxes = 0;
    moveCount = 0;
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            currentMap[y][x] = allLevels[level][y][x];

            // cari posisi awal pemain
            if (currentMap[y][x] == 'P' || currentMap[y][x] == '+') {
                playerY = y;
                playerX = x;
            }
            // hitung jumlah kotak
            if (currentMap[y][x] == 'B' || currentMap[y][x] == '*') {
                totalBoxes++;
            }
        }
        currentMap[y][MAP_WIDTH] = '\0';
    }
}

// Fungsi untuk menghitung berapa kotak yang sudah ada di target
int countBoxesOnTarget() {
    int count = 0;
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (currentMap[y][x] == '*') {
                count++;
            }
        }
    }
    return count;
}

// Cek apakah semua kotak sudah berada di target
bool checkWin() {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (currentMap[y][x] == 'B') {
                return false; // Masih ada kotak yang belum di target
            }
        }
    }
    return true;
}

// Fungsi untuk memindahkan pemain
// dy = perubahan baris (-1 atas, 1 bawah), dx = perubahan kolom (-1 kiri, 1 kanan)
void movePlayer(int dy, int dx) {
    int newX = playerX + dx;
    int newY = playerY + dy;

    // Cek tabrakan dengan dinding
    if (currentMap[newY][newX] == '#') {
        return;
    }

    // Jika di depan pemain ada Kotak ('B' atau '*')
    if (currentMap[newY][newX] == 'B' || currentMap[newY][newX] == '*') {
        int nextX = newX + dx;
        int nextY = newY + dy;

        // Cek apakah kotak bisa didorong (tujuan kosong atau target '.')
        if (currentMap[nextY][nextX] == ' ' || currentMap[nextY][nextX] == '.') {
            // Pindahkan kotak
            if (currentMap[nextY][nextX] == '.') {
                currentMap[nextY][nextX] = '*'; // Kotak sukses di target
            } else {
                currentMap[nextY][nextX] = 'B'; // Kotak biasa
            }

            // Perbarui posisi lama kotak
            if (currentMap[newY][newX] == '*') {
                currentMap[newY][newX] = '.';
            } else {
                currentMap[newY][newX] = ' ';
            }
        } else {
            return; // Kotak tidak bisa didorong (menabrak dinding/kotak lain)
        }
    }

    // Pindahkan pemain
    // Kembalikan simbol lantai sebelumnya
    if (currentMap[playerY][playerX] == '+') {
        currentMap[playerY][playerX] = '.';
    } else {
        currentMap[playerY][playerX] = ' ';
    }

    // Tempat baru pemain
    if (currentMap[newY][newX] == '.') {
        currentMap[newY][newX] = '+';
    } else {
        currentMap[newY][newX] = 'P';
    }

    playerY = newY;
    playerX = newX;

    // Langkah hanya dihitung kalau pemain benar-benar bergerak
    moveCount++;
    totalMoves++;
}

// Fungsi untuk menggambar seluruh tampilan game ke layar
void drawGame() {
    clear();

    // Judul dan nama level
    attron(COLOR_PAIR(1));
    mvprintw(1, 4, "=== GAME SOKOBAN (NCURSES) ===");
    attroff(COLOR_PAIR(1));

    mvprintw(2, 4, "Level %d dari %d : %s", currentLevel + 1, TOTAL_LEVEL, levelNames[currentLevel]);
    mvprintw(3, 4, "Panah = bergerak | r = ulang level | q = keluar");

    // Render Peta ke Layar Terminal (mulai dari baris 5)
    for (int y = 0; y < MAP_HEIGHT; y++) {
        move(5 + y, 4);
        for (int x = 0; x < MAP_WIDTH; x++) {
            char c = currentMap[y][x];

            if (c == '#') {
                attron(COLOR_PAIR(1));
                printw("#");
                attroff(COLOR_PAIR(1));
            } else if (c == 'P' || c == '+') {
                attron(COLOR_PAIR(2));
                printw("P");
                attroff(COLOR_PAIR(2));
            } else if (c == 'B') {
                attron(COLOR_PAIR(3));
                printw("B");
                attroff(COLOR_PAIR(3));
            } else if (c == '*') {
                attron(COLOR_PAIR(4));
                printw("O"); // 'O' menandakan kotak sukses di target
                attroff(COLOR_PAIR(4));
            } else if (c == '.') {
                attron(COLOR_PAIR(5));
                printw(".");
                attroff(COLOR_PAIR(5));
            } else {
                printw(" ");
            }
        }
    }

    // Informasi langkah dan jumlah kotak yang sudah benar
    mvprintw(20, 4, "Langkah: %d    Kotak di target: %d/%d", moveCount, countBoxesOnTarget(), totalBoxes);
    mvprintw(21, 4, "P = pemain | B = kotak | . = target | O = kotak sudah di target");

    refresh();
}

// Fungsi untuk menampilkan layar judul di awal game
void showTitleScreen() {
    clear();
    attron(COLOR_PAIR(1));
    mvprintw(4, 10, "=== GAME SOKOBAN (NCURSES) ===");
    attroff(COLOR_PAIR(1));
    mvprintw(6, 10, "Dorong semua kotak (B) ke titik target (.)");
    mvprintw(7, 10, "Kotak hanya bisa didorong, tidak bisa ditarik!");
    mvprintw(8, 10, "Kalau kotak nyangkut di pojok, tekan 'r' untuk ulang level.");
    mvprintw(10, 10, "Ada %d level, makin lama makin susah. Semangat!", TOTAL_LEVEL);
    mvprintw(12, 10, "Tekan tombol apa saja untuk mulai...");
    refresh();
    getch();
}

int main() {
    // Inisialisasi Ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0); // Sembunyikan kursor

    // Cek ukuran terminal, kalau terlalu kecil peta tidak muat
    if (LINES < 24 || COLS < 70) {
        endwin();
        printf("Terminal terlalu kecil! Minimal 70 kolom x 24 baris.\n");
        printf("Ukuran sekarang: %d kolom x %d baris.\n", COLS, LINES);
        return 1;
    }

    // Inisialisasi Warna
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_CYAN, COLOR_BLACK);    // Dinding
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);  // Player
        init_pair(3, COLOR_RED, COLOR_BLACK);     // Kotak (Belum di Target)
        init_pair(4, COLOR_GREEN, COLOR_BLACK);   // Kotak (Sudah di Target / Selesai)
        init_pair(5, COLOR_MAGENTA, COLOR_BLACK); // Target '.'
    }

    showTitleScreen();
    loadLevel(currentLevel);

    int ch;
    bool playing = true;

    // Perulangan utama game
    while (playing) {
        drawGame();

        // Cek Kondisi Menang
        if (checkWin()) {
            attron(COLOR_PAIR(4));
            if (currentLevel < TOTAL_LEVEL - 1) {
                // Masih ada level berikutnya
                mvprintw(23, 4, "LEVEL SELESAI dalam %d langkah! Tekan tombol apa saja...", moveCount);
                attroff(COLOR_PAIR(4));
                refresh();
                flushinp();   // buang tombol yang masih tertahan
                getch();
                currentLevel++;
                loadLevel(currentLevel);
            } else {
                // Ini level terakhir, game tamat
                mvprintw(23, 4, "SELAMAT! Semua level selesai! Total langkah: %d", totalMoves);
                attroff(COLOR_PAIR(4));
                refresh();
                flushinp();
                getch();
                playing = false;
            }
            continue; // kembali ke awal perulangan
        }

        // Baca Input Keyboard
        ch = getch();
        switch (ch) {
            case KEY_UP:
                movePlayer(-1, 0);
                break;
            case KEY_DOWN:
                movePlayer(1, 0);
                break;
            case KEY_LEFT:
                movePlayer(0, -1);
                break;
            case KEY_RIGHT:
                movePlayer(0, 1);
                break;
            case 'r':
            case 'R':
                loadLevel(currentLevel); // ulang level yang sedang dimainkan
                break;
            case 'q':
            case 'Q':
                playing = false;
                break;
        }
    }

    // Akhiri mode ncurses
    endwin();
    return 0;
}
