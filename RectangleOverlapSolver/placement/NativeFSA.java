package placement;

import java.awt.geom.Rectangle2D;
import java.util.ArrayList;

public class NativeFSA implements RectanglePlacement {
	double xgap, ygap;

	NativeFSA(double xgap, double ygap) {
		this.xgap = xgap;
		this.ygap = ygap;
	}
	public native double place(double x[],
			double y[], double w[], double h[]);

	static {
		System.loadLibrary("placement_NativeFSA");
	}

	public void place(ArrayList<RectangleView> rs) {
		ArrayList<Rectangle2D> rectangles = new ArrayList<Rectangle2D>();
		int n=rectangles.size();
		double[] x=new double[n];
		double[] y=new double[n];
		double[] w=new double[n];
		double[] h=new double[n];
		for(int i=0;i<n;i++) {
			RectangleView r=rs.get(i);
			x[i]=r.x;
			y[i]=r.y;
			w[i]=r.width+xgap;
			h[i]=r.height+ygap;
		}
		place(x,y,w,h);
		for(int i=0;i<n;i++) {
			rs.get(i).moveTo(x[i],y[i]);
		}
	}
}
