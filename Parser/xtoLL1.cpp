#include <cctype>
#include <fstream>
#include <sstream>
#include <xtoLL1.hpp>
#include <Terminal_NonTerminal.hpp>
using namespace std;

void ScanGrammarFromFile(Grammar& g, char* filename)
{
	ifstream f(filename);
	string s, t;
	bool ishead = true;
	vector<std::string> tail;
	do
	{
		getline(f, s, '\n');
		
		if(!s.compare("") || s[0] == '#')
			continue;
		else
		{
			ishead = true;
			Rule *r;
			stringstream ss(s);
			while(getline(ss, t, ' '))
			{
				if(!t.compare("-->"))
				{
					ishead = false;
					continue;
				}
				else if(!t.compare("|"))
				{
					r->RuleAddTail(tail);
					tail.clear();
				}
				else if(ishead)
				{
					r = new Rule(t);
					addNonTerminal(t);
				}
				else
				{
					tail.push_back(t);
					if(isupper(t[0]))
						addNonTerminal(t);
					else if(t.compare(EPSILON) &&
						t.compare(ENDMARKER))
						addTerminal(t);
				}
			}
			r->RuleAddTail(tail);
			tail.clear();
			g.GrammarAddRule(*r);
			//r.~Rule();
		}
	}while(f.good());
	f.close();
	g.GrammarSetStartSymbol();
}

static bool HasCycles(Grammar& g, Rule r, std::string rhead)
{
	std::string t;
	int ti;
	for(int i = 0; i < r.RuleNTails(); i++)
		if(r.RuleTail(i).size() == 1)
		{
			t = r.RuleTail(i)[0];
			if(!rhead.compare(t))
				return true;
			else if((ti = g.GrammarFindRule(t)) >= 0)
				return HasCycles(g, g.GrammarRule(ti), rhead);
		}
	return false;
}

bool HasCycles(Grammar& g)	// wrapper for the above function
{
	for(int i = 0; i < g.GrammarNRules(); i++)
		if(HasCycles(g, g.GrammarRule(i), g.GrammarRule(i).RuleHead()))
			return true;
	return false;
}

bool HasNonTerminatingProductions(Rule& r)
{
	vector<std::string> tail;
	int tail_nonterminals;
	bool headmatches;
	std::string head;
	head = r.RuleHead();
	for(int j = 0; j < r.RuleNTails(); j++)
	{
		tail = r.RuleTail(j);
		tail_nonterminals = 0;
		headmatches = false;
		for(int k = 0; k < tail.size(); k++)
		{
			if(!head.compare(tail[k]))
				headmatches = true;
			if(isNonTerminal(tail[k]))
				tail_nonterminals++;
		}
		if(tail_nonterminals == 1 &&
			headmatches &&
			!r.RuleHasTerminalProduction() &&
			r.RuleFindEpsilonProduction() == -1)
			return true;
	}
	return false;
}

bool HasNonTerminatingRules(Grammar& g)
{
	Rule r;
	for(int i = 0; i < g.GrammarNRules(); i++)
	{
		r = g.GrammarRule(i);
		if(HasNonTerminatingProductions(r))
			return true;
	}
	return false;
}

void EliminateEpsilonProductions(Grammar& g)
{
	Rule r;
	int index;
	for(int i = 0; i < g.GrammarNRules(); i++)
	{
		// find a rule (say, A) having epsilon production
		r = g.GrammarRule(i);
		if((index = r.RuleFindEpsilonProduction()) >= 0)
		{
			cerr<<"Rule has epsilon production: ";
			g.GrammarRule(i).RuleOutput();

			// remove epsilon production from A
			r.RuleRemoveTail(index);


		}
	}
}

#define PLR	ProductionsLeftRecursive
#define PNLR	ProductionsNonLeftRecursive
static vector<int> ProductionsLeftRecursive, ProductionsNonLeftRecursive;

bool HasLeftRecursion(Rule& r)
{
	PLR.clear();
	PNLR.clear();
	// Atleast one production should begin with \
	the same Non-Terminal as the head of the rule. \
	(Additionally, that production should have at \
	least one other Terminal/Non-Terminal but that \
	is guaranteed since there are no cycles.)
	// At least one production should NOT begin with \
	the same Non-Terminal as the head of the rule.
	for(int i = 0; i < r.RuleNTails(); i++)
	{
		if(r.RuleIsLeftRecursiveProduction(i))
			PLR.push_back(i);
		else
			PNLR.push_back(i);
	}
	if(PLR.size() > 0 && PNLR.size() > 0)
		return true;
	else
		return false;
}

vector<Rule> EliminateImmediateLeftRecursion(Rule& r)
{
	vector<Rule> replacementrules;
	replacementrules.push_back(Rule(r.RuleHead()));
	replacementrules.push_back(
		Rule(r.RuleHead().append(std::string("_PRIME"))));
	vector<std::string> tail;
	for(int i = 0; i < PNLR.size(); i++)
	{
		tail = r.RuleTail(PNLR[i]);
		if(!tail[0].compare(EPSILON))
			tail.clear();
		tail.push_back(replacementrules[1].RuleHead());
		replacementrules[0].RuleAddTail(tail);
	}
	for(int i = 0; i < PLR.size(); i++)
	{
		tail = r.RuleTail(PLR[i]);
		tail.erase(tail.begin() + 0);
		tail.push_back(replacementrules[1].RuleHead());
		replacementrules[1].RuleAddTail(tail);
	}
	tail.clear();
	tail.push_back(EPSILON);
	replacementrules[1].RuleAddTail(tail);
	return replacementrules;
}

void EliminateImmediateLeftRecursion(Grammar& g)
{
	Rule r;
	vector<Rule> temp, rulestoadd;
	vector<int> rulestoremove;
	int n = g.GrammarNRules(), rulestoremove_modifier = 0;
	for(int i = 0; i < n; i++)
	{
		r = g.GrammarRule(i);
		if(HasLeftRecursion(r))
		{
			temp = EliminateImmediateLeftRecursion(r);
			addNonTerminal(temp[1].RuleHead());
			rulestoremove.push_back(i);
			rulestoadd.push_back(temp[0]);
			rulestoadd.push_back(temp[1]);
		}
	}
	for(int i = 0; i < rulestoadd.size(); i++)
		g.GrammarAddRule(rulestoadd[i]);
	for(int i = 0; i < rulestoremove.size(); i++, rulestoremove_modifier++)
		g.GrammarRemoveRule(rulestoremove[i] - rulestoremove_modifier);
}

void EliminateLeftRecursion(Grammar& g)
{
	EliminateImmediateLeftRecursion(g);
}

void LeftFactorize(Grammar& g)
{
	return;
}

void EliminateDuplicateProductions(Grammar& g)
{
	Rule r;
	int rulentails, i, j, k;
	for(i = 0; i < g.GrammarNRules(); i++)
		for(j = 0, r = g.GrammarRule(i), rulentails = r.RuleNTails(); j < rulentails; j++)
			for(k = j + 1; k < rulentails; k++)
				if(r.equals(r.RuleTail(j), r.RuleTail(k)))
				{
					r.RuleRemoveTail(k);
					rulentails--;
				}
}
