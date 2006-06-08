typedef cola::constrained_majorization_layout_impl<detail::graph::edge_or_side<false, double>,cola::layout_tolerance<double> > ConstrainedMajLayout;
void output_svg(Graph g, vector<Rectangle*> &rs, char* fname);
inline double getRand(double range) {
	return range*rand()/RAND_MAX;
}

