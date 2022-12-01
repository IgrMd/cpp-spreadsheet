#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_map>

struct PositionHasher {
	size_t operator()(Position pos) const;
};

class Sheet : public SheetInterface {
	friend class Cell;
public:
	using Table = std::unordered_map<Position, std::unique_ptr<Cell>, PositionHasher>;

public:
	void SetCell(Position pos, std::string text) override;

	const CellInterface* GetCell(Position pos) const override;
	CellInterface* GetCell(Position pos) override;

	void ClearCell(Position pos) override;

	Size GetPrintableSize() const override;

	void PrintValues(std::ostream& output) const override;
	void PrintTexts(std::ostream& output) const override;

private:

	void Resize(Position pos);
	void CalculateNewSize();

	Table table_;
	Size size_;
};