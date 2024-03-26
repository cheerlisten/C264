/*
 * Tables, from openh264 
*/
#pragma once
#include <stdint.h>

enum Constants
{
    CABAC_INIT_IDC_CNT = 4,
    CABAC_CONTEXT_CNT = 460,
    QP_CNT = 51,
};
#define CTX_NA 0
#define WELS_CLIP3(iX, iY, iZ) ((iX) < (iY) ? (iY) : ((iX) > (iZ) ? (iZ) : (iX)))

// clang-format off
static constexpr uint8_t g_kRenormTable256[256] = {
  6, 6, 6, 6, 6, 6, 6, 6,
  5, 5, 5, 5, 5, 5, 5, 5,
  4, 4, 4, 4, 4, 4, 4, 4,
  4, 4, 4, 4, 4, 4, 4, 4,
  3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3,
  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1
};

const int8_t g_kiCabacGlobalContextIdx[CABAC_CONTEXT_CNT][CABAC_INIT_IDC_CNT][2] = {
  //0-10 Table 9-12
  {{20, -15}, {20, -15}, {20, -15}, {20, -15}},
  {{2, 54}, {2, 54}, {2, 54}, {2, 54}},
  {{3, 74}, {3, 74}, {3, 74}, {3, 74}},
  {{20, -15}, {20, -15}, {20, -15}, {20, -15}},
  {{2, 54}, {2, 54}, {2, 54}, {2, 54}},
  {{3, 74}, {3, 74}, {3, 74}, {3, 74}},
  {{ -28, 127}, { -28, 127}, { -28, 127}, { -28, 127}},
  {{ -23, 104}, { -23, 104}, { -23, 104}, { -23, 104}},
  {{ -6, 53}, { -6, 53}, { -6, 53}, { -6, 53}},
  {{ -1, 54}, { -1, 54}, { -1, 54}, { -1, 54}},
  {{7, 51}, {7, 51}, {7, 51}, {7, 51}},
  //11-23 Table 9-13
  {{CTX_NA, CTX_NA}, {23, 33}, {22, 25}, {29, 16}},
  {{CTX_NA, CTX_NA}, {23, 2}, {34, 0}, {25, 0}},
  {{CTX_NA, CTX_NA}, {21, 0}, {16, 0}, {14, 0}},
  {{CTX_NA, CTX_NA}, {1, 9}, { -2, 9}, { -10, 51}},
  {{CTX_NA, CTX_NA}, {0, 49}, {4, 41}, { -3, 62}},
  {{CTX_NA, CTX_NA}, { -37, 118}, { -29, 118}, { -27, 99}},
  {{CTX_NA, CTX_NA}, {5, 57}, {2, 65}, {26, 16}},
  {{CTX_NA, CTX_NA}, { -13, 78}, { -6, 71}, { -4, 85}},
  {{CTX_NA, CTX_NA}, { -11, 65}, { -13, 79}, { -24, 102}},
  {{CTX_NA, CTX_NA}, {1, 62}, {5, 52}, {5, 57}},
  {{CTX_NA, CTX_NA}, {12, 49}, {9, 50}, {6, 57}},
  {{CTX_NA, CTX_NA}, { -4, 73}, { -3, 70}, { -17, 73}},
  {{CTX_NA, CTX_NA}, {17, 50}, {10, 54}, {14, 57}},
  //24-39 Table9-14
  {{CTX_NA, CTX_NA}, {18, 64}, {26, 34}, {20, 40}},
  {{CTX_NA, CTX_NA}, {9, 43}, {19, 22}, {20, 10}},
  {{CTX_NA, CTX_NA}, {29, 0}, {40, 0}, {29, 0}},
  {{CTX_NA, CTX_NA}, {26, 67}, {57, 2}, {54, 0}},
  {{CTX_NA, CTX_NA}, {16, 90}, {41, 36}, {37, 42}},
  {{CTX_NA, CTX_NA}, {9, 104}, {26, 69}, {12, 97}},
  {{CTX_NA, CTX_NA}, { -46, 127}, { -45, 127}, { -32, 127}},
  {{CTX_NA, CTX_NA}, { -20, 104}, { -15, 101}, { -22, 117}},
  {{CTX_NA, CTX_NA}, {1, 67}, { -4, 76}, { -2, 74}},
  {{CTX_NA, CTX_NA}, { -13, 78}, { -6, 71}, { -4, 85}},
  {{CTX_NA, CTX_NA}, { -11, 65}, { -13, 79}, { -24, 102}},
  {{CTX_NA, CTX_NA}, {1, 62}, {5, 52}, {5, 57}},
  {{CTX_NA, CTX_NA}, { -6, 86}, {6, 69}, { -6, 93}},
  {{CTX_NA, CTX_NA}, { -17, 95}, { -13, 90}, { -14, 88}},
  {{CTX_NA, CTX_NA}, { -6, 61}, {0, 52}, { -6, 44}},
  {{CTX_NA, CTX_NA}, {9, 45}, {8, 43}, {4, 55}},
  //40-53 Table 9-15
  {{CTX_NA, CTX_NA}, { -3, 69}, { -2, 69}, { -11, 89}},
  {{CTX_NA, CTX_NA}, { -6, 81}, { -5, 82}, { -15, 103}},
  {{CTX_NA, CTX_NA}, { -11, 96}, { -10, 96}, { -21, 116}},
  {{CTX_NA, CTX_NA}, {6, 55}, {2, 59}, {19, 57}},
  {{CTX_NA, CTX_NA}, {7, 67}, {2, 75}, {20, 58}},
  {{CTX_NA, CTX_NA}, { -5, 86}, { -3, 87}, {4, 84}},
  {{CTX_NA, CTX_NA}, {2, 88}, { -3, 100}, {6, 96}},
  {{CTX_NA, CTX_NA}, {0, 58}, {1, 56}, {1, 63}},
  {{CTX_NA, CTX_NA}, { -3, 76}, { -3, 74}, { -5, 85}},
  {{CTX_NA, CTX_NA}, { -10, 94}, { -6, 85}, { -13, 106}},
  {{CTX_NA, CTX_NA}, {5, 54}, {0, 59}, {5, 63}},
  {{CTX_NA, CTX_NA}, {4, 69}, { -3, 81}, {6, 75}},
  {{CTX_NA, CTX_NA}, { -3, 81}, { -7, 86}, { -3, 90}},
  {{CTX_NA, CTX_NA}, {0, 88}, { -5, 95}, { -1, 101}},
  //54-59 Table 9-16
  {{CTX_NA, CTX_NA}, { -7, 67}, { -1, 66}, {3, 55}},
  {{CTX_NA, CTX_NA}, { -5, 74}, { -1, 77}, { -4, 79}},
  {{CTX_NA, CTX_NA}, { -4, 74}, {1, 70}, { -2, 75}},
  {{CTX_NA, CTX_NA}, { -5, 80}, { -2, 86}, { -12, 97}},
  {{CTX_NA, CTX_NA}, { -7, 72}, { -5, 72}, { -7, 50}},
  {{CTX_NA, CTX_NA}, {1, 58}, {0, 61}, {1, 60}},
  //60-69 Table 9-17
  {{0, 41}, {0, 41}, {0, 41}, {0, 41}},
  {{0, 63}, {0, 63}, {0, 63}, {0, 63}},
  {{0, 63}, {0, 63}, {0, 63}, {0, 63}},
  {{0, 63}, {0, 63}, {0, 63}, {0, 63}},
  {{ -9, 83}, { -9, 83}, { -9, 83}, { -9, 83}},
  {{4, 86}, {4, 86}, {4, 86}, {4, 86}},
  {{0, 97}, {0, 97}, {0, 97}, {0, 97}},
  {{ -7, 72}, { -7, 72}, { -7, 72}, { -7, 72}},
  {{13, 41}, {13, 41}, {13, 41}, {13, 41}},
  {{3, 62}, {3, 62}, {3, 62}, {3, 62}},
  //70-104 Table 9-18
  {{0, 11}, {0, 45}, {13, 15}, {7, 34}},
  {{1, 55}, { -4, 78}, {7, 51}, { -9, 88}},
  {{0, 69}, { -3, 96}, {2, 80}, { -20, 127}},
  {{ -17, 127}, { -27, 126}, { -39, 127}, { -36, 127}},
  {{ -13, 102}, { -28, 98}, { -18, 91}, { -17, 91}},
  {{0, 82}, { -25, 101}, { -17, 96}, { -14, 95}},
  {{ -7, 74}, { -23, 67}, { -26, 81}, { -25, 84}},
  {{ -21, 107}, { -28, 82}, { -35, 98}, { -25, 86}},
  {{ -27, 127}, { -20, 94}, { -24, 102}, { -12, 89}},
  {{ -31, 127}, { -16, 83}, { -23, 97}, { -17, 91}},
  {{ -24, 127}, { -22, 110}, { -27, 119}, { -31, 127}},
  {{ -18, 95}, { -21, 91}, { -24, 99}, { -14, 76}},
  {{ -27, 127}, { -18, 102}, { -21, 110}, { -18, 103}},
  {{ -21, 114}, { -13, 93}, { -18, 102}, { -13, 90}},
  {{ -30, 127}, { -29, 127}, { -36, 127}, { -37, 127}},
  {{ -17, 123}, { -7, 92}, {0, 80}, {11, 80}},
  {{ -12, 115}, { -5, 89}, { -5, 89}, {5, 76}},
  {{ -16, 122}, { -7, 96}, { -7, 94}, {2, 84}},
  {{ -11, 115}, { -13, 108}, { -4, 92}, {5, 78}},
  {{ -12, 63}, { -3, 46}, {0, 39}, { -6, 55}},
  {{ -2, 68}, { -1, 65}, {0, 65}, {4, 61}},
  {{ -15, 84}, { -1, 57}, { -15, 84}, { -14, 83}},
  {{ -13, 104}, { -9, 93}, { -35, 127}, { -37, 127}},
  {{ -3, 70}, { -3, 74}, { -2, 73}, { -5, 79}},
  {{ -8, 93}, { -9, 92}, { -12, 104}, { -11, 104}},
  {{ -10, 90}, { -8, 87}, { -9, 91}, { -11, 91}},
  {{ -30, 127}, { -23, 126}, { -31, 127}, { -30, 127}},
  {{ -1, 74}, {5, 54}, {3, 55}, {0, 65}},
  {{ -6, 97}, {6, 60}, {7, 56}, { -2, 79}},
  {{ -7, 91}, {6, 59}, {7, 55}, {0, 72}},
  {{ -20, 127}, {6, 69}, {8, 61}, { -4, 92}},
  {{ -4, 56}, { -1, 48}, { -3, 53}, { -6, 56}},
  {{ -5, 82}, {0, 68}, {0, 68}, {3, 68}},
  {{ -7, 76}, { -4, 69}, { -7, 74}, { -8, 71}},
  {{ -22, 125}, { -8, 88}, { -9, 88}, { -13, 98}},
  //105-165 Table 9-19
  {{ -7, 93}, { -2, 85}, { -13, 103}, { -4, 86}},
  {{ -11, 87}, { -6, 78}, { -13, 91}, { -12, 88}},
  {{ -3, 77}, { -1, 75}, { -9, 89}, { -5, 82}},
  {{ -5, 71}, { -7, 77}, { -14, 92}, { -3, 72}},
  {{ -4, 63}, {2, 54}, { -8, 76}, { -4, 67}},
  {{ -4, 68}, {5, 50}, { -12, 87}, { -8, 72}},
  {{ -12, 84}, { -3, 68}, { -23, 110}, { -16, 89}},
  {{ -7, 62}, {1, 50}, { -24, 105}, { -9, 69}},
  {{ -7, 65}, {6, 42}, { -10, 78}, { -1, 59}},
  {{8, 61}, { -4, 81}, { -20, 112}, {5, 66}},
  {{5, 56}, {1, 63}, { -17, 99}, {4, 57}},
  {{ -2, 66}, { -4, 70}, { -78, 127}, { -4, 71}},
  {{1, 64}, {0, 67}, { -70, 127}, { -2, 71}},
  {{0, 61}, {2, 57}, { -50, 127}, {2, 58}},
  {{ -2, 78}, { -2, 76}, { -46, 127}, { -1, 74}},
  {{1, 50}, {11, 35}, { -4, 66}, { -4, 44}},
  {{7, 52}, {4, 64}, { -5, 78}, { -1, 69}},
  {{10, 35}, {1, 61}, { -4, 71}, {0, 62}},
  {{0, 44}, {11, 35}, { -8, 72}, { -7, 51}},
  {{11, 38}, {18, 25}, {2, 59}, { -4, 47}},
  {{1, 45}, {12, 24}, { -1, 55}, { -6, 42}},
  {{0, 46}, {13, 29}, { -7, 70}, { -3, 41}},
  {{5, 44}, {13, 36}, { -6, 75}, { -6, 53}},
  {{31, 17}, { -10, 93}, { -8, 89}, {8, 76}},
  {{1, 51}, { -7, 73}, { -34, 119}, { -9, 78}},
  {{7, 50}, { -2, 73}, { -3, 75}, { -11, 83}},
  {{28, 19}, {13, 46}, {32, 20}, {9, 52}},
  {{16, 33}, {9, 49}, {30, 22}, {0, 67}},
  {{14, 62}, { -7, 100}, { -44, 127}, { -5, 90}},
  {{ -13, 108}, {9, 53}, {0, 54}, {1, 67}},
  {{ -15, 100}, {2, 53}, { -5, 61}, { -15, 72}},
  {{ -13, 101}, {5, 53}, {0, 58}, { -5, 75}},
  {{ -13, 91}, { -2, 61}, { -1, 60}, { -8, 80}},
  {{ -12, 94}, {0, 56}, { -3, 61}, { -21, 83}},
  {{ -10, 88}, {0, 56}, { -8, 67}, { -21, 64}},
  {{ -16, 84}, { -13, 63}, { -25, 84}, { -13, 31}},
  {{ -10, 86}, { -5, 60}, { -14, 74}, { -25, 64}},
  {{ -7, 83}, { -1, 62}, { -5, 65}, { -29, 94}},
  {{ -13, 87}, {4, 57}, {5, 52}, {9, 75}},
  {{ -19, 94}, { -6, 69}, {2, 57}, {17, 63}},
  {{1, 70}, {4, 57}, {0, 61}, { -8, 74}},
  {{0, 72}, {14, 39}, { -9, 69}, { -5, 35}},
  {{ -5, 74}, {4, 51}, { -11, 70}, { -2, 27}},
  {{18, 59}, {13, 68}, {18, 55}, {13, 91}},
  {{ -8, 102}, {3, 64}, { -4, 71}, {3, 65}},
  {{ -15, 100}, {1, 61}, {0, 58}, { -7, 69}},
  {{0, 95}, {9, 63}, {7, 61}, {8, 77}},
  {{ -4, 75}, {7, 50}, {9, 41}, { -10, 66}},
  {{2, 72}, {16, 39}, {18, 25}, {3, 62}},
  {{ -11, 75}, {5, 44}, {9, 32}, { -3, 68}},
  {{ -3, 71}, {4, 52}, {5, 43}, { -20, 81}},
  {{15, 46}, {11, 48}, {9, 47}, {0, 30}},
  {{ -13, 69}, { -5, 60}, {0, 44}, {1, 7}},
  {{0, 62}, { -1, 59}, {0, 51}, { -3, 23}},
  {{0, 65}, {0, 59}, {2, 46}, { -21, 74}},
  {{21, 37}, {22, 33}, {19, 38}, {16, 66}},
  {{ -15, 72}, {5, 44}, { -4, 66}, { -23, 124}},
  {{9, 57}, {14, 43}, {15, 38}, {17, 37}},
  {{16, 54}, { -1, 78}, {12, 42}, {44, -18}},
  {{0, 62}, {0, 60}, {9, 34}, {50, -34}},
  {{12, 72}, {9, 69}, {0, 89}, { -22, 127}},
  //166-226 Table 9-20
  {{24, 0}, {11, 28}, {4, 45}, {4, 39}},
  {{15, 9}, {2, 40}, {10, 28}, {0, 42}},
  {{8, 25}, {3, 44}, {10, 31}, {7, 34}},
  {{13, 18}, {0, 49}, {33, -11}, {11, 29}},
  {{15, 9}, {0, 46}, {52, -43}, {8, 31}},
  {{13, 19}, {2, 44}, {18, 15}, {6, 37}},
  {{10, 37}, {2, 51}, {28, 0}, {7, 42}},
  {{12, 18}, {0, 47}, {35, -22}, {3, 40}},
  {{6, 29}, {4, 39}, {38, -25}, {8, 33}},
  {{20, 33}, {2, 62}, {34, 0}, {13, 43}},
  {{15, 30}, {6, 46}, {39, -18}, {13, 36}},
  {{4, 45}, {0, 54}, {32, -12}, {4, 47}},
  {{1, 58}, {3, 54}, {102, -94}, {3, 55}},
  {{0, 62}, {2, 58}, {0, 0}, {2, 58}},
  {{7, 61}, {4, 63}, {56, -15}, {6, 60}},
  {{12, 38}, {6, 51}, {33, -4}, {8, 44}},
  {{11, 45}, {6, 57}, {29, 10}, {11, 44}},
  {{15, 39}, {7, 53}, {37, -5}, {14, 42}},
  {{11, 42}, {6, 52}, {51, -29}, {7, 48}},
  {{13, 44}, {6, 55}, {39, -9}, {4, 56}},
  {{16, 45}, {11, 45}, {52, -34}, {4, 52}},
  {{12, 41}, {14, 36}, {69, -58}, {13, 37}},
  {{10, 49}, {8, 53}, {67, -63}, {9, 49}},
  {{30, 34}, { -1, 82}, {44, -5}, {19, 58}},
  {{18, 42}, {7, 55}, {32, 7}, {10, 48}},
  {{10, 55}, { -3, 78}, {55, -29}, {12, 45}},
  {{17, 51}, {15, 46}, {32, 1}, {0, 69}},
  {{17, 46}, {22, 31}, {0, 0}, {20, 33}},
  {{0, 89}, { -1, 84}, {27, 36}, {8, 63}},
  {{26, -19}, {25, 7}, {33, -25}, {35, -18}},
  {{22, -17}, {30, -7}, {34, -30}, {33, -25}},
  {{26, -17}, {28, 3}, {36, -28}, {28, -3}},
  {{30, -25}, {28, 4}, {38, -28}, {24, 10}},
  {{28, -20}, {32, 0}, {38, -27}, {27, 0}},
  {{33, -23}, {34, -1}, {34, -18}, {34, -14}},
  {{37, -27}, {30, 6}, {35, -16}, {52, -44}},
  {{33, -23}, {30, 6}, {34, -14}, {39, -24}},
  {{40, -28}, {32, 9}, {32, -8}, {19, 17}},
  {{38, -17}, {31, 19}, {37, -6}, {31, 25}},
  {{33, -11}, {26, 27}, {35, 0}, {36, 29}},
  {{40, -15}, {26, 30}, {30, 10}, {24, 33}},
  {{41, -6}, {37, 20}, {28, 18}, {34, 15}},
  {{38, 1}, {28, 34}, {26, 25}, {30, 20}},
  {{41, 17}, {17, 70}, {29, 41}, {22, 73}},
  {{30, -6}, {1, 67}, {0, 75}, {20, 34}},
  {{27, 3}, {5, 59}, {2, 72}, {19, 31}},
  {{26, 22}, {9, 67}, {8, 77}, {27, 44}},
  {{37, -16}, {16, 30}, {14, 35}, {19, 16}},
  {{35, -4}, {18, 32}, {18, 31}, {15, 36}},
  {{38, -8}, {18, 35}, {17, 35}, {15, 36}},
  {{38, -3}, {22, 29}, {21, 30}, {21, 28}},
  {{37, 3}, {24, 31}, {17, 45}, {25, 21}},
  {{38, 5}, {23, 38}, {20, 42}, {30, 20}},
  {{42, 0}, {18, 43}, {18, 45}, {31, 12}},
  {{35, 16}, {20, 41}, {27, 26}, {27, 16}},
  {{39, 22}, {11, 63}, {16, 54}, {24, 42}},
  {{14, 48}, {9, 59}, {7, 66}, {0, 93}},
  {{27, 37}, {9, 64}, {16, 56}, {14, 56}},
  {{21, 60}, { -1, 94}, {11, 73}, {15, 57}},
  {{12, 68}, { -2, 89}, {10, 67}, {26, 38}},
  {{2, 97}, { -9, 108}, { -10, 116}, { -24, 127}},
  //227-275 Table 9-21
  {{ -3, 71}, { -6, 76}, { -23, 112}, { -24, 115}},
  {{ -6, 42}, { -2, 44}, { -15, 71}, { -22, 82}},
  {{ -5, 50}, {0, 45}, { -7, 61}, { -9, 62}},
  {{ -3, 54}, {0, 52}, {0, 53}, {0, 53}},
  {{ -2, 62}, { -3, 64}, { -5, 66}, {0, 59}},
  {{0, 58}, { -2, 59}, { -11, 77}, { -14, 85}},
  {{1, 63}, { -4, 70}, { -9, 80}, { -13, 89}},
  {{ -2, 72}, { -4, 75}, { -9, 84}, { -13, 94}},
  {{ -1, 74}, { -8, 82}, { -10, 87}, { -11, 92}},
  {{ -9, 91}, { -17, 102}, { -34, 127}, { -29, 127}},
  {{ -5, 67}, { -9, 77}, { -21, 101}, { -21, 100}},
  {{ -5, 27}, {3, 24}, { -3, 39}, { -14, 57}},
  {{ -3, 39}, {0, 42}, { -5, 53}, { -12, 67}},
  {{ -2, 44}, {0, 48}, { -7, 61}, { -11, 71}},
  {{0, 46}, {0, 55}, { -11, 75}, { -10, 77}},
  {{ -16, 64}, { -6, 59}, { -15, 77}, { -21, 85}},
  {{ -8, 68}, { -7, 71}, { -17, 91}, { -16, 88}},
  {{ -10, 78}, { -12, 83}, { -25, 107}, { -23, 104}},
  {{ -6, 77}, { -11, 87}, { -25, 111}, { -15, 98}},
  {{ -10, 86}, { -30, 119}, { -28, 122}, { -37, 127}},
  {{ -12, 92}, {1, 58}, { -11, 76}, { -10, 82}},
  {{ -15, 55}, { -3, 29}, { -10, 44}, { -8, 48}},
  {{ -10, 60}, { -1, 36}, { -10, 52}, { -8, 61}},
  {{ -6, 62}, {1, 38}, { -10, 57}, { -8, 66}},
  {{ -4, 65}, {2, 43}, { -9, 58}, { -7, 70}},
  {{ -12, 73}, { -6, 55}, { -16, 72}, { -14, 75}},
  {{ -8, 76}, {0, 58}, { -7, 69}, { -10, 79}},
  {{ -7, 80}, {0, 64}, { -4, 69}, { -9, 83}},
  {{ -9, 88}, { -3, 74}, { -5, 74}, { -12, 92}},
  {{ -17, 110}, { -10, 90}, { -9, 86}, { -18, 108}},
  {{ -11, 97}, {0, 70}, {2, 66}, { -4, 79}},
  {{ -20, 84}, { -4, 29}, { -9, 34}, { -22, 69}},
  {{ -11, 79}, {5, 31}, {1, 32}, { -16, 75}},
  {{ -6, 73}, {7, 42}, {11, 31}, { -2, 58}},
  {{ -4, 74}, {1, 59}, {5, 52}, {1, 58}},
  {{ -13, 86}, { -2, 58}, { -2, 55}, { -13, 78}},
  {{ -13, 96}, { -3, 72}, { -2, 67}, { -9, 83}},
  {{ -11, 97}, { -3, 81}, {0, 73}, { -4, 81}},
  {{ -19, 117}, { -11, 97}, { -8, 89}, { -13, 99}},
  {{ -8, 78}, {0, 58}, {3, 52}, { -13, 81}},
  {{ -5, 33}, {8, 5}, {7, 4}, { -6, 38}},
  {{ -4, 48}, {10, 14}, {10, 8}, { -13, 62}},
  {{ -2, 53}, {14, 18}, {17, 8}, { -6, 58}},
  {{ -3, 62}, {13, 27}, {16, 19}, { -2, 59}},
  {{ -13, 71}, {2, 40}, {3, 37}, { -16, 73}},
  {{ -10, 79}, {0, 58}, { -1, 61}, { -10, 76}},
  {{ -12, 86}, { -3, 70}, { -5, 73}, { -13, 86}},
  {{ -13, 90}, { -6, 79}, { -1, 70}, { -9, 83}},
  {{ -14, 97}, { -8, 85}, { -4, 78}, { -10, 87}},
  //276 no use
  {{CTX_NA, CTX_NA}, {CTX_NA, CTX_NA}, {CTX_NA, CTX_NA}, {CTX_NA, CTX_NA}},
  //277-337 Table 9-22
  {{ -6, 93}, { -13, 106}, { -21, 126}, { -22, 127}},
  {{ -6, 84}, { -16, 106}, { -23, 124}, { -25, 127}},
  {{ -8, 79}, { -10, 87}, { -20, 110}, { -25, 120}},
  {{0, 66}, { -21, 114}, { -26, 126}, { -27, 127}},
  {{ -1, 71}, { -18, 110}, { -25, 124}, { -19, 114}},
  {{0, 62}, { -14, 98}, { -17, 105}, { -23, 117}},
  {{ -2, 60}, { -22, 110}, { -27, 121}, { -25, 118}},
  {{ -2, 59}, { -21, 106}, { -27, 117}, { -26, 117}},
  {{ -5, 75}, { -18, 103}, { -17, 102}, { -24, 113}},
  {{ -3, 62}, { -21, 107}, { -26, 117}, { -28, 118}},
  {{ -4, 58}, { -23, 108}, { -27, 116}, { -31, 120}},
  {{ -9, 66}, { -26, 112}, { -33, 122}, { -37, 124}},
  {{ -1, 79}, { -10, 96}, { -10, 95}, { -10, 94}},
  {{0, 71}, { -12, 95}, { -14, 100}, { -15, 102}},
  {{3, 68}, { -5, 91}, { -8, 95}, { -10, 99}},
  {{10, 44}, { -9, 93}, { -17, 111}, { -13, 106}},
  {{ -7, 62}, { -22, 94}, { -28, 114}, { -50, 127}},
  {{15, 36}, { -5, 86}, { -6, 89}, { -5, 92}},
  {{14, 40}, {9, 67}, { -2, 80}, {17, 57}},
  {{16, 27}, { -4, 80}, { -4, 82}, { -5, 86}},
  {{12, 29}, { -10, 85}, { -9, 85}, { -13, 94}},
  {{1, 44}, { -1, 70}, { -8, 81}, { -12, 91}},
  {{20, 36}, {7, 60}, { -1, 72}, { -2, 77}},
  {{18, 32}, {9, 58}, {5, 64}, {0, 71}},
  {{5, 42}, {5, 61}, {1, 67}, { -1, 73}},
  {{1, 48}, {12, 50}, {9, 56}, {4, 64}},
  {{10, 62}, {15, 50}, {0, 69}, { -7, 81}},
  {{17, 46}, {18, 49}, {1, 69}, {5, 64}},
  {{9, 64}, {17, 54}, {7, 69}, {15, 57}},
  {{ -12, 104}, {10, 41}, { -7, 69}, {1, 67}},
  {{ -11, 97}, {7, 46}, { -6, 67}, {0, 68}},
  {{ -16, 96}, { -1, 51}, { -16, 77}, { -10, 67}},
  {{ -7, 88}, {7, 49}, { -2, 64}, {1, 68}},
  {{ -8, 85}, {8, 52}, {2, 61}, {0, 77}},
  {{ -7, 85}, {9, 41}, { -6, 67}, {2, 64}},
  {{ -9, 85}, {6, 47}, { -3, 64}, {0, 68}},
  {{ -13, 88}, {2, 55}, {2, 57}, { -5, 78}},
  {{4, 66}, {13, 41}, { -3, 65}, {7, 55}},
  {{ -3, 77}, {10, 44}, { -3, 66}, {5, 59}},
  {{ -3, 76}, {6, 50}, {0, 62}, {2, 65}},
  {{ -6, 76}, {5, 53}, {9, 51}, {14, 54}},
  {{10, 58}, {13, 49}, { -1, 66}, {15, 44}},
  {{ -1, 76}, {4, 63}, { -2, 71}, {5, 60}},
  {{ -1, 83}, {6, 64}, { -2, 75}, {2, 70}},
  {{ -7, 99}, { -2, 69}, { -1, 70}, { -2, 76}},
  {{ -14, 95}, { -2, 59}, { -9, 72}, { -18, 86}},
  {{2, 95}, {6, 70}, {14, 60}, {12, 70}},
  {{0, 76}, {10, 44}, {16, 37}, {5, 64}},
  {{ -5, 74}, {9, 31}, {0, 47}, { -12, 70}},
  {{0, 70}, {12, 43}, {18, 35}, {11, 55}},
  {{ -11, 75}, {3, 53}, {11, 37}, {5, 56}},
  {{1, 68}, {14, 34}, {12, 41}, {0, 69}},
  {{0, 65}, {10, 38}, {10, 41}, {2, 65}},
  {{ -14, 73}, { -3, 52}, {2, 48}, { -6, 74}},
  {{3, 62}, {13, 40}, {12, 41}, {5, 54}},
  {{4, 62}, {17, 32}, {13, 41}, {7, 54}},
  {{ -1, 68}, {7, 44}, {0, 59}, { -6, 76}},
  {{ -13, 75}, {7, 38}, {3, 50}, { -11, 82}},
  {{11, 55}, {13, 50}, {19, 40}, { -2, 77}},
  {{5, 64}, {10, 57}, {3, 66}, { -2, 77}},
  {{12, 70}, {26, 43}, {18, 50}, {25, 42}},
  //338-398 Table9-23
  {{15, 6}, {14, 11}, {19, -6}, {17, -13}},
  {{6, 19}, {11, 14}, {18, -6}, {16, -9}},
  {{7, 16}, {9, 11}, {14, 0}, {17, -12}},
  {{12, 14}, {18, 11}, {26, -12}, {27, -21}},
  {{18, 13}, {21, 9}, {31, -16}, {37, -30}},
  {{13, 11}, {23, -2}, {33, -25}, {41, -40}},
  {{13, 15}, {32, -15}, {33, -22}, {42, -41}},
  {{15, 16}, {32, -15}, {37, -28}, {48, -47}},
  {{12, 23}, {34, -21}, {39, -30}, {39, -32}},
  {{13, 23}, {39, -23}, {42, -30}, {46, -40}},
  {{15, 20}, {42, -33}, {47, -42}, {52, -51}},
  {{14, 26}, {41, -31}, {45, -36}, {46, -41}},
  {{14, 44}, {46, -28}, {49, -34}, {52, -39}},
  {{17, 40}, {38, -12}, {41, -17}, {43, -19}},
  {{17, 47}, {21, 29}, {32, 9}, {32, 11}},
  {{24, 17}, {45, -24}, {69, -71}, {61, -55}},
  {{21, 21}, {53, -45}, {63, -63}, {56, -46}},
  {{25, 22}, {48, -26}, {66, -64}, {62, -50}},
  {{31, 27}, {65, -43}, {77, -74}, {81, -67}},
  {{22, 29}, {43, -19}, {54, -39}, {45, -20}},
  {{19, 35}, {39, -10}, {52, -35}, {35, -2}},
  {{14, 50}, {30, 9}, {41, -10}, {28, 15}},
  {{10, 57}, {18, 26}, {36, 0}, {34, 1}},
  {{7, 63}, {20, 27}, {40, -1}, {39, 1}},
  {{ -2, 77}, {0, 57}, {30, 14}, {30, 17}},
  {{ -4, 82}, { -14, 82}, {28, 26}, {20, 38}},
  {{ -3, 94}, { -5, 75}, {23, 37}, {18, 45}},
  {{9, 69}, { -19, 97}, {12, 55}, {15, 54}},
  {{ -12, 109}, { -35, 125}, {11, 65}, {0, 79}},
  {{36, -35}, {27, 0}, {37, -33}, {36, -16}},
  {{36, -34}, {28, 0}, {39, -36}, {37, -14}},
  {{32, -26}, {31, -4}, {40, -37}, {37, -17}},
  {{37, -30}, {27, 6}, {38, -30}, {32, 1}},
  {{44, -32}, {34, 8}, {46, -33}, {34, 15}},
  {{34, -18}, {30, 10}, {42, -30}, {29, 15}},
  {{34, -15}, {24, 22}, {40, -24}, {24, 25}},
  {{40, -15}, {33, 19}, {49, -29}, {34, 22}},
  {{33, -7}, {22, 32}, {38, -12}, {31, 16}},
  {{35, -5}, {26, 31}, {40, -10}, {35, 18}},
  {{33, 0}, {21, 41}, {38, -3}, {31, 28}},
  {{38, 2}, {26, 44}, {46, -5}, {33, 41}},
  {{33, 13}, {23, 47}, {31, 20}, {36, 28}},
  {{23, 35}, {16, 65}, {29, 30}, {27, 47}},
  {{13, 58}, {14, 71}, {25, 44}, {21, 62}},
  {{29, -3}, {8, 60}, {12, 48}, {18, 31}},
  {{26, 0}, {6, 63}, {11, 49}, {19, 26}},
  {{22, 30}, {17, 65}, {26, 45}, {36, 24}},
  {{31, -7}, {21, 24}, {22, 22}, {24, 23}},
  {{35, -15}, {23, 20}, {23, 22}, {27, 16}},
  {{34, -3}, {26, 23}, {27, 21}, {24, 30}},
  {{34, 3}, {27, 32}, {33, 20}, {31, 29}},
  {{36, -1}, {28, 23}, {26, 28}, {22, 41}},
  {{34, 5}, {28, 24}, {30, 24}, {22, 42}},
  {{32, 11}, {23, 40}, {27, 34}, {16, 60}},
  {{35, 5}, {24, 32}, {18, 42}, {15, 52}},
  {{34, 12}, {28, 29}, {25, 39}, {14, 60}},
  {{39, 11}, {23, 42}, {18, 50}, {3, 78}},
  {{30, 29}, {19, 57}, {12, 70}, { -16, 123}},
  {{34, 26}, {22, 53}, {21, 54}, {21, 53}},
  {{29, 39}, {22, 61}, {14, 71}, {22, 56}},
  {{19, 66}, {11, 86}, {11, 83}, {25, 61}},
  {{31, 21}, {12, 40}, {25, 32}, {21, 33}},
  {{31, 31}, {11, 51}, {21, 49}, {19, 50}},
  {{25, 50}, {14, 59}, {21, 54}, {17, 61}},
  //402-459 Table 9-24
  {{ -17, 120}, { -4, 79}, { -5, 85}, { -3, 78}},
  {{ -20, 112}, { -7, 71}, { -6, 81}, { -8, 74}},
  {{ -18, 114}, { -5, 69}, { -10, 77}, { -9, 72}},
  {{ -11, 85}, { -9, 70}, { -7, 81}, { -10, 72}},
  {{ -15, 92}, { -8, 66}, { -17, 80}, { -18, 75}},
  {{ -14, 89}, { -10, 68}, { -18, 73}, { -12, 71}},
  {{ -26, 71}, { -19, 73}, { -4, 74}, { -11, 63}},
  {{ -15, 81}, { -12, 69}, { -10, 83}, { -5, 70}},
  {{ -14, 80}, { -16, 70}, { -9, 71}, { -17, 75}},
  {{0, 68}, { -15, 67}, { -9, 67}, { -14, 72}},
  {{ -14, 70}, { -20, 62}, { -1, 61}, { -16, 67}},
  {{ -24, 56}, { -19, 70}, { -8, 66}, { -8, 53}},
  {{ -23, 68}, { -16, 66}, { -14, 66}, { -14, 59}},
  {{ -24, 50}, { -22, 65}, {0, 59}, { -9, 52}},
  {{ -11, 74}, { -20, 63}, {2, 59}, { -11, 68}},
  {{23, -13}, {9, -2}, {17, -10}, {9, -2}},
  {{26, -13}, {26, -9}, {32, -13}, {30, -10}},
  {{40, -15}, {33, -9}, {42, -9}, {31, -4}},
  {{49, -14}, {39, -7}, {49, -5}, {33, -1}},
  {{44, 3}, {41, -2}, {53, 0}, {33, 7}},
  {{45, 6}, {45, 3}, {64, 3}, {31, 12}},
  {{44, 34}, {49, 9}, {68, 10}, {37, 23}},
  {{33, 54}, {45, 27}, {66, 27}, {31, 38}},
  {{19, 82}, {36, 59}, {47, 57}, {20, 64}},
  {{ -3, 75}, { -6, 66}, { -5, 71}, { -9, 71}},
  {{ -1, 23}, { -7, 35}, {0, 24}, { -7, 37}},
  {{1, 34}, { -7, 42}, { -1, 36}, { -8, 44}},
  {{1, 43}, { -8, 45}, { -2, 42}, { -11, 49}},
  {{0, 54}, { -5, 48}, { -2, 52}, { -10, 56}},
  {{ -2, 55}, { -12, 56}, { -9, 57}, { -12, 59}},
  {{0, 61}, { -6, 60}, { -6, 63}, { -8, 63}},
  {{1, 64}, { -5, 62}, { -4, 65}, { -9, 67}},
  {{0, 68}, { -8, 66}, { -4, 67}, { -6, 68}},
  {{ -9, 92}, { -8, 76}, { -7, 82}, { -10, 79}},
  {{ -14, 106}, { -5, 85}, { -3, 81}, { -3, 78}},
  {{ -13, 97}, { -6, 81}, { -3, 76}, { -8, 74}},
  {{ -15, 90}, { -10, 77}, { -7, 72}, { -9, 72}},
  {{ -12, 90}, { -7, 81}, { -6, 78}, { -10, 72}},
  {{ -18, 88}, { -17, 80}, { -12, 72}, { -18, 75}},
  {{ -10, 73}, { -18, 73}, { -14, 68}, { -12, 71}},
  {{ -9, 79}, { -4, 74}, { -3, 70}, { -11, 63}},
  {{ -14, 86}, { -10, 83}, { -6, 76}, { -5, 70}},
  {{ -10, 73}, { -9, 71}, { -5, 66}, { -17, 75}},
  {{ -10, 70}, { -9, 67}, { -5, 62}, { -14, 72}},
  {{ -10, 69}, { -1, 61}, {0, 57}, { -16, 67}},
  {{ -5, 66}, { -8, 66}, { -4, 61}, { -8, 53}},
  {{ -9, 64}, { -14, 66}, { -9, 60}, { -14, 59}},
  {{ -5, 58}, {0, 59}, {1, 54}, { -9, 52}},
  {{2, 59}, {2, 59}, {2, 58}, { -11, 68}},
  {{21, -10}, {21, -13}, {17, -10}, {9, -2}},
  {{24, -11}, {33, -14}, {32, -13}, {30, -10}},
  {{28, -8}, {39, -7}, {42, -9}, {31, -4}},
  {{28, -1}, {46, -2}, {49, -5}, {33, -1}},
  {{29, 3}, {51, 2}, {53, 0}, {33, 7}},
  {{29, 9}, {60, 6}, {64, 3}, {31, 12}},
  {{35, 20}, {61, 17}, {68, 10}, {37, 23}},
  {{29, 36}, {55, 34}, {66, 27}, {31, 38}},
  {{14, 67}, {42, 62}, {47, 57}, {20, 64}},
};

/* Table 9-44 - Specification of rangeTabLPS depending on pStateIdx and qCodIRangeIdx */

const uint8_t g_kuiCabacRangeLps[64][4] = {
  { 128, 176, 208, 240}, { 128, 167, 197, 227}, { 128, 158, 187, 216}, { 123, 150, 178, 205}, { 116, 142, 169, 195}, { 111, 135, 160, 185}, { 105, 128, 152, 175}, { 100, 122, 144, 166},
  {  95, 116, 137, 158}, {  90, 110, 130, 150}, {  85, 104, 123, 142}, {  81,  99, 117, 135}, {  77,  94, 111, 128}, {  73,  89, 105, 122}, {  69,  85, 100, 116}, {  66,  80,  95, 110},
  {  62,  76,  90, 104}, {  59,  72,  86,  99}, {  56,  69,  81,  94}, {  53,  65,  77,  89}, {  51,  62,  73,  85}, {  48,  59,  69,  80}, {  46,  56,  66,  76}, {  43,  53,  63,  72},
  {  41,  50,  59,  69}, {  39,  48,  56,  65}, {  37,  45,  54,  62}, {  35,  43,  51,  59}, {  33,  41,  48,  56}, {  32,  39,  46,  53}, {  30,  37,  43,  50}, {  29,  35,  41,  48},
  {  27,  33,  39,  45}, {  26,  31,  37,  43}, {  24,  30,  35,  41}, {  23,  28,  33,  39}, {  22,  27,  32,  37}, {  21,  26,  30,  35}, {  20,  24,  29,  33}, {  19,  23,  27,  31},
  {  18,  22,  26,  30}, {  17,  21,  25,  28}, {  16,  20,  23,  27}, {  15,  19,  22,  25}, {  14,  18,  21,  24}, {  14,  17,  20,  23}, {  13,  16,  19,  22}, {  12,  15,  18,  21},
  {  12,  14,  17,  20}, {  11,  14,  16,  19}, {  11,  13,  15,  18}, {  10,  12,  15,  17}, {  10,  12,  14,  16}, {   9,  11,  13,  15}, {   9,  11,  12,  14}, {   8,  10,  12,  14},
  {   8,   9,  11,  13}, {   7,   9,  11,  12}, {   7,   9,  10,  12}, {   7,   8,  10,  11}, {   6,   8,   9,  11}, {   6,   7,   9,  10}, {   6,   7,   8,   9}, {   2,   2,   2,   2}
};

/* Table 9-45 - State transition table */

const uint8_t g_kuiStateTransTable[64][2] = {

  {0, 1}, {0, 2}, {1, 3}, {2, 4}, {2, 5}, {4, 6}, {4, 7}, {5, 8}, {6, 9}, {7, 10},

  {8, 11}, {9, 12}, {9, 13}, {11, 14}, {11, 15}, {12, 16}, {13, 17}, {13, 18}, {15, 19}, {15, 20},

  {16, 21}, {16, 22}, {18, 23}, {18, 24}, {19, 25}, {19, 26}, {21, 27}, {21, 28}, {22, 29}, {22, 30},

  {23, 31}, {24, 32}, {24, 33}, {25, 34}, {26, 35}, {26, 36}, {27, 37}, {27, 38}, {28, 39}, {29, 40},

  {29, 41}, {30, 42}, {30, 43}, {30, 44}, {31, 45}, {32, 46}, {32, 47}, {33, 48}, {33, 49}, {33, 50},

  {34, 51}, {34, 52}, {35, 53}, {35, 54}, {35, 55}, {36, 56}, {36, 57}, {36, 58}, {37, 59}, {37, 60},

  {37, 61}, {38, 62}, {38, 62}, {63, 63}

};

// clang-format on
