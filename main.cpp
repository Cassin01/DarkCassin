#include "parsecpp.cpp"
#include <numeric>

int toInt(const std::string &s) {
  int ret;
  std::istringstream(s) >> ret;
  return ret;
}

Parser<int> eval(
  const Parser<int> &m,
  const Parser<std::list<std::function<int (int)>>> &fs) {
  return [=](Source *s) {
    int x = m(s);
    auto xs = fs(s);
    return std::accumulate(xs.begin(), xs.end(), x,
      [](int x, const std::function<int (int)> &f) { return f(x); });
  };
}

Parser<std::function<int (int)>> apply( // 特殊化
  const std::function<int (int, int)> &f, const Parser<int> &p) {
  return apply<int, int, int>(f, p);
}

auto number = apply<int, std::string>(toInt, many1(digit));

extern Parser<int> factor_; // 前方宣言
Parser<int> factor = [](Source *s) { return factor_(s); }; // ダミーのラッパー

// term = number, {("*", number) | ("/", number)}
auto term = eval(factor, many(
     char1('*') >> apply([](int x, int y) { return y * x; }, factor)
  || char1('/') >> apply([](int x, int y) { return y / x; }, factor)
));

// expr = term, {("+", term) | ("-", term)}
auto expr = eval(term, many(
     char1('+') >> apply([](int x, int y) { return y + x; }, term)
  || char1('-') >> apply([](int x, int y) { return y - x; }, term)
));

// factor = ("(", expr, ")" | number
//Parser<int> factor_ = char1('(') >> expr << char1(')') || number; // 実体

// factor = factor = [spaces], ("(", expr, ")") | number, [spaces]
Parser<int> factor_ = spaces
                   >> (char1('(') >> expr << char1(')') || number)
                   << spaces;

int main() {
  parseTest(number, "123");
  parseTest(expr,   "1+2");
  parseTest(expr,   "123");
  parseTest(expr,   "1+2+3");
  parseTest(expr,   "1-2-3");
  parseTest(expr,   "1-2+3");
  parseTest(expr,   "2*3+4"); //
  parseTest(expr,   "2+3*4");
  parseTest(expr,   "100 / 10 / 2");
  parseTest(expr,   "(2+3)*4");
}
