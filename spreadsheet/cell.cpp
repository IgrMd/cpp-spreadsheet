#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

using namespace std::string_literals;

using Value = CellInterface::Value;

// ----- Cell -----
Cell::Cell(SheetInterface& owner)
	: owner_(owner)
	, impl_(std::make_unique<EmptyImpl>()) {
}

void Cell::ClearCahe() {
	if (impl_->HasCahe()) {
		for (Cell* parent : parents_) {
			parent->ClearCahe();
		}
	}
	impl_->ClearCache();
}

void Cell::AddParent(Cell* parent) {
	parents_.insert(parent);
}

void Cell::AddChilds() {
	childs_.clear();
	auto ref_cells = GetReferencedCells();
	for (auto cell : ref_cells) {
		if (Cell* child = dynamic_cast<Cell*>(owner_.GetCell(cell))) {
			childs_.insert(child);
		} else {
			owner_.SetCell(cell, "");
			childs_.insert(dynamic_cast<Cell*>(owner_.GetCell(cell)));
		}
	}
}

static std::unordered_set<Cell*> MakeChildList(std::vector<Position> ref_cells, SheetInterface& owner) {
	std::unordered_set<Cell*> childs;
	for (auto cell : ref_cells) {
		if (Cell* child = dynamic_cast<Cell*>(owner.GetCell(cell))) {
			childs.insert(child);
		} else {
			owner.SetCell(cell, "");
			childs.insert(dynamic_cast<Cell*>(owner.GetCell(cell)));
		}
	}
	return childs;
}

void Cell::AddParentness() {
	for (Cell* child : childs_) {
		child->AddParent(this);
	}
}

void Cell::RemoveParentness() {
	for (Cell* child : childs_) {
		child->RemoveParent(this);
	}
}

void Cell::RemoveParent(Cell* parent) {
	parents_.erase(parent);
}

bool Cell::CircularDependencyFound(Cell* target, const std::unordered_set<Cell*>& childs) const {
	if (childs.size() == 0) {
		return false;
	}
	if (childs.count(target)) {
		return true;
	}
	for (Cell* child : childs) {
		if (CircularDependencyFound(target, child->childs_)) {
			return true;
		}
	}
	return false;
}

void Cell::Set(std::string text) {
	if (text.empty()) {
		impl_ = std::make_unique<EmptyImpl>();
		return;
	}
	if (text.size() > 1 && text.front() == FORMULA_SIGN) {
		auto nem_impl = std::make_unique<FormulaImpl>(text.substr(1, text.size() - 1));
		auto new_childs = MakeChildList(nem_impl->GetReferencedCells(), owner_);
		if (CircularDependencyFound(this, new_childs)) {
			throw CircularDependencyException("");
		};
		impl_ = std::move(nem_impl);
		RemoveParentness();
		childs_ = std::move(new_childs);
		AddParentness();
		ClearCahe();
		return;
	}
	impl_ = std::make_unique<TextImpl>(text);
}

void Cell::Clear() {
	impl_ = std::make_unique<EmptyImpl>();
}

Value Cell::GetValue() const {
	assert(impl_);
	return impl_->GetValue(owner_);
}

std::string Cell::GetText() const {
	assert(impl_);
	return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
	return std::move(impl_->GetReferencedCells());
}

// ----- Impl -----
std::vector<Position> Cell::Impl::GetReferencedCells() const {
	return {};
}

bool Cell::Impl::HasCahe() const { 
	return false;
}

// ----- EmptyImpl -----
Value Cell::EmptyImpl::GetValue(const SheetInterface& owner) const {
	return Value();
}

std::string Cell::EmptyImpl::GetText() const {
	return std::string();
}

// ----- TextImpl -----
Cell::TextImpl::TextImpl(std::string text)
	: text_(std::move(text)) {}

Value Cell::TextImpl::GetValue(const SheetInterface& owner) const {
	return text_.front() == ESCAPE_SIGN ? text_.substr(1, text_.size() - 1) : text_;
}

std::string Cell::TextImpl::GetText() const {
	return text_;
}

// ----- FormulaImpl -----
Cell::FormulaImpl::FormulaImpl(std::string text)
	: formula_(ParseFormula(std::move(text))) {}

Value Cell::FormulaImpl::GetValue(const SheetInterface& owner) const {
	assert(formula_);
	if (cache_.has_value()) {
		return cache_.value();
	}
	FormulaInterface::Value result = formula_->Evaluate(owner);
	if (std::holds_alternative<double>(result)) {
		cache_ = std::get<double>(result);
	}
	if (std::holds_alternative<FormulaError>(result)) {
		cache_ = std::get<FormulaError>(result);
	}
	return cache_.value();
}

std::string Cell::FormulaImpl::GetText() const {
	assert(formula_);
	return std::string{ FORMULA_SIGN }.append(formula_->GetExpression());
}

void Cell::FormulaImpl::ClearCache() {
	cache_.reset();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
	return std::move(formula_->GetReferencedCells());
}

bool Cell::FormulaImpl::HasCahe() const {
	return cache_.has_value();
}