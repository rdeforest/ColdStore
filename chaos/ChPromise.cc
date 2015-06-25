#include "ChPromise.hh"

Slot ChPromise::call(Slot &args)
{
    // have we already been evaluated?
    if (!evaluator)
	return expression;
    
    evaluator->ChPush(expression);
    evaluator->ChEval();
    expression = evaluator->ChPop();
    evaluator = NULL;
    return expression;
}

Slot ChPromise::toconstruct() const
{
    // have we already been evaluated?
    if (!evaluator)
	return expression->toconstruct();
    
    return new List(-2, (Data*)(evaluator->toconstruct()), (Data*)(expression->toconstruct()));
}

ChPromise::~ChPromise()
{}
