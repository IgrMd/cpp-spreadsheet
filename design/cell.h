#pragma once

#include "common.h"
#include "formula.h"

#include <optional>
#include <functional>
#include <unordered_map>
#include <unordered_set>

class Cell final : public CellInterface {
public:
	Cell(SheetInterface& owner);

	void Set(std::string text);
	void Clear();

	Value GetValue() const override;
	std::string GetText() const override;
	std::vector<Position> GetReferencedCells() const override;

private:

	class Impl {
	public:
		virtual Value GetValue(const SheetInterface& owner) const = 0;
		virtual std::string GetText() const = 0;
		virtual void ClearCache() {};
		virtual bool HasCahe() const;
		virtual std::vector<Position> GetReferencedCells() const;
		virtual ~Impl() = default;
	};

	class EmptyImpl final : public Impl {
	public:
		virtual Value GetValue(const SheetInterface& owner) const override;
		virtual std::string GetText() const override;
	};

	class TextImpl final : public Impl {
	public:
		TextImpl(std::string text);
		virtual Value GetValue(const SheetInterface& owner) const override;
		virtual std::string GetText() const override;
	private:
		std::string text_;
	};

	class FormulaImpl final : public Impl {
	public:
		FormulaImpl(std::string text);
		virtual Value GetValue(const SheetInterface& owner) const override;
		virtual std::string GetText() const override;
		virtual void ClearCache() override;
		virtual bool HasCahe() const override ;
		virtual std::vector<Position> GetReferencedCells() const;

	private:
		std::unique_ptr<FormulaInterface> formula_;
		mutable std::optional<Value> cache_;
	};

	void ClearCahe();

	void AddChilds();
	void AddParentness();
	void AddParent(Cell* parent);

	void RemoveParentness();
	void RemoveParent(Cell* parent);

	bool CircularDependencyFound(Cell* target, const std::unordered_set<Cell*>& childs) const;

	SheetInterface& owner_;
	std::unique_ptr<Impl> impl_;
	std::unordered_set<Cell*> childs_;
	std::unordered_set<Cell*> parents_;

};