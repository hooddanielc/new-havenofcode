namespace hoc {
  class db_result_t;

  class db_row_t final {
    friend db_result_t;
    public:
      const char * operator[](int col) const {
        return PQgetvalue(res, row, col);
      }

    private:
      int row;
      PGresult *res;
      db_row_t(PGresult *res, int row) : row(row), res(res) {};
  };
}
