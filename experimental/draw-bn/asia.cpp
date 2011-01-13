#include <vector>
#include <string>

#include <boost/spirit/core.hpp>
#include <boost/spirit/actor/push_back_actor.hpp>
#include <iostream>
#include <vector>
#include <string>

using namespace std;
using namespace boost::spirit;

bool
parse_numbers(char const* str, vector<double>& v)
{
    return parse(str,

        //  Begin grammar
        (
            real_p[push_back_a(v)] >> *(',' >> real_p[push_back_a(v)])
        )
        ,
        //  End grammar

        space_p).full;
}

void
construct_graph(vector<Node*> &nn) {
	Node* VisitAsia = new Node("Visit To Asia");
	VisitAsia->states.push_back("Visit");
	VisitAsia->states.push_back("No Visit");
	assert (parse_numbers("0.01, 0.99", VisitAsia->probs));
	assert (parse_numbers("0.01, 0.99", VisitAsia->belief));
	nn.push_back(VisitAsia);
	
	Node* Tuberculosis = new Node("Tuberculosis");
	Tuberculosis->states.push_back("Present");
	Tuberculosis->states.push_back("Absent");
	Tuberculosis->parents.push_back(VisitAsia);
	assert (parse_numbers("0.05,0.95,0.01,0.99", Tuberculosis->probs));
	assert (parse_numbers("0.0104,0.9896", Tuberculosis->belief));
	nn.push_back(Tuberculosis);
	
	Node* Smoking = new Node("Smoking");
	Smoking->states.push_back("Smoker");
	Smoking->states.push_back("NonSmoker");
	assert (parse_numbers("0.5,0.5", Smoking->probs));
	assert (parse_numbers("0.5,0.5", Smoking->belief));
	nn.push_back(Smoking);
	
	Node* Cancer = new Node("Lung Cancer");
	Cancer->states.push_back("Present");
	Cancer->states.push_back("Absent");
	Cancer->parents.push_back(Smoking);
	assert (parse_numbers("0.1,0.9,0.01,0.99", Cancer->probs));
	assert (parse_numbers("0.055,0.945", Cancer->belief));
	nn.push_back(Cancer);
	
	Node* TbOrCa = new Node("Tuberculosis or Cancer");
	TbOrCa->states.push_back("True");
	TbOrCa->states.push_back("False");
	TbOrCa->parents.push_back(Tuberculosis);
	TbOrCa->parents.push_back(Cancer);
	assert (parse_numbers("1,0,1,0,1,0,0,1", TbOrCa->probs));
	assert (parse_numbers("0.064828,0.935172", TbOrCa->belief));
	nn.push_back(TbOrCa);

	Node* XRay = new Node("XRay Result");
	XRay->states.push_back("Abnormal");
	XRay->states.push_back("Normal");
	XRay->parents.push_back(TbOrCa);
	assert (parse_numbers("0.98,0.02,0.05,0.95", XRay->probs));
	assert (parse_numbers("0.11029,0.88971", XRay->belief));
	nn.push_back(XRay);

	Node* Bronchitis = new Node("Bronchitis");
	Bronchitis->states.push_back("Present");
	Bronchitis->states.push_back("Absent");
	Bronchitis->parents.push_back(Smoking);
	assert (parse_numbers("0.6,0.4,0.3,0.7", Bronchitis->probs));
	assert (parse_numbers("0.45,0.55", Bronchitis->belief));
	nn.push_back(Bronchitis);

	Node* Dyspnea = new Node("Dyspnea");
	Dyspnea->states.push_back("Present");
	Dyspnea->states.push_back("Absent");
	Dyspnea->parents.push_back(TbOrCa);
	Dyspnea->parents.push_back(Bronchitis);
	assert (parse_numbers("0.9,0.1,0.7,0.3,0.8,0.2,0.1,0.9", Dyspnea->probs));
	assert (parse_numbers("0.435971, 0.564029", Dyspnea->belief));
	nn.push_back(Dyspnea);
}
