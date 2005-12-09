package placement;

import java.awt.geom.Rectangle2D;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;

public class FSA implements RectanglePlacement {
	class Vec {
		double x, y;
	}

	double xgap, ygap;

	FSA(double xgap, double ygap) {
		this.xgap = xgap;
		this.ygap = ygap;
	}

	ArrayList<Rectangle2D> rectangles;

	Vec force(Rectangle2D vi, Rectangle2D vj) {
		Vec f = new Vec();
		double dx = vj.getCenterX() - vi.getCenterX();
		double dy = vj.getCenterY() - vi.getCenterY();
		double adx = Math.abs(dx);
		double ady = Math.abs(dy);
		double gij = dy / dx;
		double Gij = (vi.getHeight() + vj.getHeight())
				/ (vi.getWidth() + vj.getWidth());
		if (Gij >= gij && gij > 0 || -Gij <= gij && gij < 0 || gij == 0) {
			// vi and vj touch with y-direction boundaries
			f.x = dx / adx * ((vi.getWidth() + vj.getWidth()) / 2.0 - adx);
			f.y = f.x * gij;
		}
		if (Gij < gij && gij > 0 || -Gij > gij && gij < 0) {
			// vi and vj touch with x-direction boundaries
			f.y = dy / ady * ((vi.getHeight() + vj.getHeight()) / 2.0 - ady);
			f.x = f.y / gij;
		}
		return f;
	}

	Vec force2(Rectangle2D vi, Rectangle2D vj) {
		Vec f = new Vec();
		double dx = vj.getCenterX() - vi.getCenterX();
		double dy = vj.getCenterY() - vi.getCenterY();
		double gij = dy / dx;
		if (vi.intersects(vj)) {
			f.x = (vi.getWidth() + vj.getWidth()) / 2.0
					- (vj.getCenterX() - vi.getCenterX());
			f.y = (vi.getHeight() + vj.getHeight()) / 2.0
					- (vj.getCenterY() - vi.getCenterY());
			// in the x dimension
			if (f.x > f.y && gij != 0) {
				f.x = f.y / gij;
			}
			f.x = Math.max(f.x, 0);
			f.y = Math.max(f.y, 0);
		}
		return f;
	}

	Comparator<Rectangle2D> xComparator = new Comparator<Rectangle2D>() {
		public int compare(Rectangle2D u, Rectangle2D v) {
			if (u.getCenterX() > v.getCenterX()) {
				return 1;
			}
			if (u.getCenterX() < v.getCenterX()) {
				return -1;
			}
			return 0;
		}
	};

	void horizontal() {
		Collections.sort(rectangles, xComparator);
		int i = 0, n = rectangles.size();
		while (i < n) {
			// x_i = x_{i+1} = ... = x_k
			int k = i;
			Rectangle2D u = rectangles.get(i);
			for (int j = i; j < n; j++) {
				Rectangle2D v = rectangles.get(j);
				if (u.getCenterX() == v.getCenterX()) {
					u = v;
					k = j;
				} else {
					break;
				}
			}
			// delta = max(0, max{f.x(m,j)|i<=m<=k<j<n})
			double delta = 0;
			for (int m = i; m <= k; m++) {
				for (int j = k + 1; j < n; j++) {
					Vec f = force(rectangles.get(m), rectangles.get(j));
					if (f.x > delta) {
						delta = f.x;
					}
				}
			}
			for (int j = k + 1; j < n; j++) {
				Rectangle2D r = rectangles.get(j);
				r.setRect(r.getMinX() + delta, r.getMinY(), r.getWidth(), r
						.getHeight());
			}
			i = k + 1;
		}

	}

	double horizontalImproved() {
		Collections.sort(rectangles, xComparator);
		int i = 0, n = rectangles.size();
		Rectangle2D lmin = rectangles.get(0);
		double sigma = 0, x0 = lmin.getCenterX();
		double[] gamma = new double[rectangles.size()];
		double[] x = new double[rectangles.size()];
		while (i < n) {
			Rectangle2D u = rectangles.get(i);
			int k = i;
			for (int j = i + 1; j < n; j++) {
				Rectangle2D v = rectangles.get(j);
				if (u.getCenterX() == v.getCenterX()) {
					u = v;
					k = j;
				} else {
					break;
				}
			}
			double g = 0;
			if (u.getCenterX() > x0) {
				for (int m = i; m <= k; m++) {
					double ggg = 0;
					for (int j = 0; j < i; j++) {
						Vec f = force(rectangles.get(j), rectangles.get(m));
						ggg = Math.max(f.x + gamma[j], ggg);
					}
					Rectangle2D v = rectangles.get(m);
					double gg = 0;
					if (v.getMinX() + ggg < lmin.getMinX()) {
						gg = sigma;
					} else {
						gg = ggg;
					}
					g = Math.max(g, gg);
				}
			}
			for (int m = i; m <= k; m++) {
				gamma[m] = g;
				Rectangle2D r = rectangles.get(m);
				x[m] = r.getMinX() + g;
				if (r.getMinX() < lmin.getMinX()) {
					lmin = r;
				}
			}
			// delta = max(0, max{f.x(m,j)|i<=m<=k<j<n})
			double delta = 0;
			for (int m = i; m <= k; m++) {
				for (int j = k + 1; j < n; j++) {
					Vec f = force(rectangles.get(m), rectangles.get(j));
					if (f.x > delta) {
						delta = f.x;
					}
				}
			}
			sigma += delta;
			i = k + 1;
		}
		double cost = 0;
		for (i = 0; i < n; i++) {
			Rectangle2D r = rectangles.get(i);
			double oldPos = r.getMinX();
			r.setRect(x[i], r.getMinY(), r.getWidth(), r.getHeight());
			double newPos = r.getMinX();

			double diff = oldPos - newPos;
			cost += diff * diff;
		}
		return cost;
	}

	Comparator<Rectangle2D> yComparator = new Comparator<Rectangle2D>() {

		public int compare(Rectangle2D u, Rectangle2D v) {
			if (u.getCenterY() > v.getCenterY()) {
				return 1;
			}
			if (u.getCenterY() < v.getCenterY()) {
				return -1;
			}
			return 0;
		}

	};

	void vertical() {
		Collections.sort(rectangles, yComparator);
		int i = 0, n = rectangles.size();
		while (i < n) {
			// y_i = y_{i+1} = ... = y_k
			int k = i;
			Rectangle2D u = rectangles.get(i);
			for (int j = i; j < n; j++) {
				Rectangle2D v = rectangles.get(j);
				if (u.getCenterY() == v.getCenterY()) {
					u = v;
					k = j;
				} else {
					break;
				}
			}
			// delta = max(0, max{f.y(m,j)|i<=m<=k<j<n})
			double delta = 0;
			for (int m = i; m <= k; m++) {
				for (int j = k + 1; j < n; j++) {
					Vec f = force2(rectangles.get(m), rectangles.get(j));
					if (f.y > delta) {
						delta = f.y;
					}
				}
			}
			for (int j = k + 1; j < n; j++) {
				Rectangle2D r = rectangles.get(j);
				r.setRect(r.getMinX(), r.getMinY() + delta, r.getWidth(), r
						.getHeight());
			}
			i = k + 1;
		}

	}

	double verticalImproved() {
		Collections.sort(rectangles, yComparator);
		int i = 0, n = rectangles.size();
		Rectangle2D lmin = rectangles.get(0);
		double sigma = 0, y0 = lmin.getCenterY();
		double[] gamma = new double[rectangles.size()];
		double[] y = new double[rectangles.size()];
		while (i < n) {
			Rectangle2D u = rectangles.get(i);
			int k = i;
			for (int j = i + 1; j < n; j++) {
				Rectangle2D v = rectangles.get(j);
				if (u.getCenterY() == v.getCenterY()) {
					u = v;
					k = j;
				} else {
					break;
				}
			}
			double g = 0;
			if (u.getCenterY() > y0) {
				for (int m = i; m <= k; m++) {
					double ggg = 0;
					for (int j = 0; j < i; j++) {
						Vec f = force2(rectangles.get(j), rectangles.get(m));
						ggg = Math.max(f.y + gamma[j], ggg);
					}
					Rectangle2D v = rectangles.get(m);
					double gg = 0;
					if (v.getMinY() + ggg < lmin.getMinY()) {
						gg = sigma;
					} else {
						gg = ggg;
					}
					g = Math.max(g, gg);
				}
			}
			for (int m = i; m <= k; m++) {
				gamma[m] = g;
				Rectangle2D r = rectangles.get(m);
				y[m] = r.getMinY() + g;
				if (r.getMinY() < lmin.getMinY()) {
					lmin = r;
				}
			}
			// delta = max(0, max{f.x(m,j)|i<=m<=k<j<n})
			double delta = 0;
			for (int m = i; m <= k; m++) {
				for (int j = k + 1; j < n; j++) {
					Vec f = force(rectangles.get(m), rectangles.get(j));
					if (f.y > delta) {
						delta = f.y;
					}
				}
			}
			sigma += delta;
			i = k + 1;
		}

		double cost = 0;
		for (i = 0; i < n; i++) {
			Rectangle2D r = rectangles.get(i);
			double oldPos = r.getMinY();
			r.setRect(r.getMinX(), y[i], r.getWidth(), r.getHeight());
			double newPos = r.getMinY();
			double diff = oldPos - newPos;
			cost += diff * diff;
		}
		return cost;
	}

	boolean ntve = true;

	public void place(ArrayList<RectangleView> rs) {
		if (ntve) {
			NativeFSA nfsa = new NativeFSA(xgap, ygap);
			nfsa.place(rs);
		} else {
			rectangles = new ArrayList<Rectangle2D>();
			for(RectangleView r:rs) {
				rectangles.add(new Rectangle2D.Double(r.x,r.y,r.width,r.height));
			}
			// TODO This isn't going to work... need to operate directly on RectangleView
			// the following is a hack but it's the easiest way to get the gaps
			// working
			for (Rectangle2D r : rectangles) {
				r.setRect(r.getMinX(), r.getMinY(), r.getWidth() + xgap, r
						.getHeight()
						+ ygap);
			}
			long t0 = System.currentTimeMillis();
			double cost = horizontalImproved();
			long t1 = System.currentTimeMillis();
			System.out.println("PFS horizontal: cost=" + cost + " time="
					+ (t1 - t0));
			cost = verticalImproved();
			long t2 = System.currentTimeMillis();
			System.out.println("PFS vertical: cost=" + cost + " time="
					+ (t2 - t1));
			System.out.println("PFS total: time=" + (t2 - t0));
			for (Rectangle2D r : rectangles) {
				r.setRect(r.getMinX(), r.getMinY(), r.getWidth() - xgap, r
						.getHeight()
						- ygap);
			}
		}
	}

}
