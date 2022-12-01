#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
	return output << fe.ToString();
}

namespace {
class Formula : public FormulaInterface {
public:
	explicit Formula(std::string expression)
		: ast_(ParseFormulaAST(expression))
		, ref_cells_(ast_.GetCells().begin(), ast_.GetCells().end()) {
		auto it = std::unique(ref_cells_.begin(), ref_cells_.end());
		ref_cells_.erase(it, ref_cells_.end());
	}
	Value Evaluate(const SheetInterface& sheet) const override {
		try {
			return ast_.Execute(sheet);
		} catch (FormulaErrorException& fe) {
			return fe.GetError();
		}
	}
	std::string GetExpression() const override {
		std::ostringstream os;
		ast_.PrintFormula(os);
		return os.str();
	}
	std::vector<Position> GetReferencedCells() const override {
		return ref_cells_;
	}

private:
	FormulaAST ast_;
	std::vector<Position> ref_cells_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
	try {
		return std::make_unique<Formula>(std::move(expression));
	} catch (std::exception& e) {
		throw FormulaException(e.what());
	}
}