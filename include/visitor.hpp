#pragma once

template <typename... Types>
struct Visitor;

template <typename First, typename... Types>
struct Visitor<First, Types...> : public Visitor<Types...>
{
    using::Visitor<Types...>::visit;

    virtual void visit(const First &visitable) = 0;
};

template <typename First>
struct Visitor <First>
{
    virtual void visit(const First &visitable) = 0;
};

template <typename VisitorType>
struct Visitable
{
    virtual void accept(VisitorType *visitor) const = 0;
};

template <typename VisitableType, typename VisitorType>
struct VisitableImpl : public virtual Visitable<VisitorType>
{
    virtual void accept(VisitorType* visitor) const
    {
        visitor->visit(*static_cast<const VisitableType*>(this));
    }
};
