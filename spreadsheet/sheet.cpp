#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <cassert>
#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

size_t PositionHasher::operator()(Position pos) const {
	return std::hash<int>{}(pos.row) + std::hash<int>{}(pos.col);
}

void Sheet::Resize(Position pos) {
	size_.rows = std::max(size_.rows, pos.row + 1);
	size_.cols = std::max(size_.cols, pos.col + 1);
}

static void CheckPos(Position pos) {
	if (!pos.IsValid()) {
		throw InvalidPositionException("Invalid position");
	}
}

void Sheet::SetCell(Position pos, std::string text) {
	CheckPos(pos);
	if (!table_.count(pos)) {
		table_[pos] = std::make_unique<Cell>(*this);
	}
	table_.at(pos)->Set(std::move(text));
	Resize(pos);
}

const CellInterface* Sheet::GetCell(Position pos) const {
	CheckPos(pos);
	if (table_.count(pos)) {
		return table_.at(pos).get();
	}
	return nullptr;
}

CellInterface* Sheet::GetCell(Position pos) {
	CheckPos(pos);
	if (table_.count(pos)) {
		return table_.at(pos).get();
	}
	return nullptr;
}

void Sheet::ClearCell(Position pos) {
	CheckPos(pos);
	if (!table_.count(pos)) {
		return;
	}
	table_.erase(pos);
	if (size_ == Size{ pos.row + 1, pos.col + 1 }) {
		CalculateNewSize();
	}
}

void Sheet::CalculateNewSize() {
	size_ = { 0,0 };
	for (const auto& [pos, cell] : table_) {
		Resize(pos);
	}
}

Size Sheet::GetPrintableSize() const {
	return size_;
}


static std::ostream& operator<<(std::ostream& out, const CellInterface::Value& value) {
	if (std::holds_alternative<double>(value)) {
		out << std::get<double>(value);
	}
	if (std::holds_alternative<std::string>(value)) {
		out << std::get<std::string>(value);
	}
	if (std::holds_alternative<FormulaError>(value)) {
		out << std::get<FormulaError>(value).ToString();
	}
	return out;
}

template<typename Printer>
static void TablePrinter(std::ostream& output, const Sheet::Table& table,
	Size print_size, Printer printer) {
	for (int row = 0; row < print_size.rows; ++row) {
		bool is_first_cell = true;
		for (int col = 0; col < print_size.cols; ++col) {
			if (!is_first_cell) {
				output << '\t';
			}
			if (Position pos{ row, col }; table.count(pos)) {
				printer(output, table.at(pos));
			}
			is_first_cell = false;
		}
		output << '\n';
	}
}

void Sheet::PrintValues(std::ostream& output) const {
	TablePrinter(output, table_, size_,
		[](std::ostream& output, auto& cell) { output << cell->GetValue(); }
	);
}


void Sheet::PrintTexts(std::ostream& output) const {
	TablePrinter(output, table_, size_,
		[](std::ostream& output, auto& cell) { output << cell->GetText(); }
	);
}

std::unique_ptr<SheetInterface> CreateSheet() {
	return std::make_unique<Sheet>();
}