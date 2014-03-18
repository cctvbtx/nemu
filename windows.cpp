#include <memory>
#include "qemu_manage.h"

QManager::TemplateWindow::TemplateWindow(int height, int width, int starty, int startx) {
  height_ = height;
  width_ = width;
  starty_ = starty;
  startx_ = startx;

  getmaxyx(stdscr, row, col);
}

void QManager::TemplateWindow::Init() {
  start_color();
  use_default_colors();
  window = newwin(height_, width_, starty_, startx_);
  keypad(window, TRUE);
}

void QManager::MainWindow::Print() {
  clear();
  border(0,0,0,0,0,0,0,0);
  mvprintw(1, 1, "Use arrow keys to go up and down, Press enter to select a choice, F10 - exit");
  refresh();
}

void QManager::VmWindow::Print() {
  border(0,0,0,0,0,0,0,0);
  mvprintw(1, 1, "F1 - help, F10 - main menu");
  refresh();
}

QManager::VmInfoWindow::VmInfoWindow(const std::string &guest, int height, int width,
  int starty, int startx) : TemplateWindow(height, width, starty, startx) {
    guest_ = guest;
    title_ = guest_ + " info";
}

void QManager::VmInfoWindow::Print() {
  clear();
  border(0,0,0,0,0,0,0,0);
  mvprintw(1, (col - title_.size())/2, title_.c_str());
  getch();
  refresh();
  clear();
}

QManager::AddVmWindow::AddVmWindow(const std::string &dbf, int height, int width,
  int starty, int startx) : TemplateWindow(height, width, starty, startx) {
    dbf_ = dbf;
}

void QManager::AddVmWindow::Print() {
  sql_last_vnc = "select vnc from lastval";
  sql_last_mac = "select mac from lastval";

  VectorString q_arch(list_arch());
  MapString u_dev(list_usb());

  clear();
  border(0,0,0,0,0,0,0,0);
  mvprintw(1, 1, "F10 - finish, F2 - save");
  curs_set(1);

  std::unique_ptr<QemuDb> db(new QemuDb(dbf_));
  v_last_vnc = db->SelectQuery(sql_last_vnc);
  v_last_mac = db->SelectQuery(sql_last_mac);

  last_vnc = std::stoi(v_last_vnc[0]);
  last_vnc++;

  for(int i(0); i<=10; i++) {
    field[i] = new_field(1, 35, i*2, 1, 0, 0);
    set_field_back(field[i], A_UNDERLINE); 
  }

  init_pair(1, COLOR_BLACK, COLOR_WHITE);
  wbkgd(window, COLOR_PAIR(1));

  set_field_type(field[0], TYPE_ALNUM, 0);
  //set_field_type(field[1], TYPE_ENUM, q_arch, false, false);
  
  getch();
  curs_set(0);
}

void QManager::HelpWindow::Print() {
  line = 1;
  window_ = newwin(height_, width_, starty_, startx_);
  keypad(window_, TRUE);
  box(window_, 0, 0);

  msg_.push_back("\"r\" - start guest");
  msg_.push_back("\"c\" - connect to guest via vnc");
  msg_.push_back("\"f\" - force stop guest");
  msg_.push_back("\"d\" - delete guest");
  msg_.push_back("\"e\" - edit guest settings");
  msg_.push_back("\"l\" - clone guest");

  for(auto &msg : msg_) {
    mvwprintw(window_, line, 1, "%s", msg.c_str());
    line++;
  }

  wrefresh(window_);
  wgetch(window_);
}

QManager::PopupWarning::PopupWarning(const std::string &msg, int height,
 int width, int starty, int startx) : TemplateWindow(height, width, starty, startx) {
    msg_ = msg;
}

int QManager::PopupWarning::Print(WINDOW *window) {
  window_ = window;
  box(window_, 0, 0);
  mvwprintw(window_, 1, 1, "%s", msg_.c_str());
  wrefresh(window_);
  ch_ = wgetch(window_);

  return ch_;
}

QManager::MenuList::MenuList(WINDOW *window, uint32_t &highlight) {
  window_ = window;
  highlight_ = highlight;
}

QManager::VmList::VmList(WINDOW *menu_window, uint32_t &highlight, const std::string &vmdir)
        : MenuList(menu_window, highlight) {
  vmdir_ = vmdir;
  init_pair(1, COLOR_WHITE, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
}
