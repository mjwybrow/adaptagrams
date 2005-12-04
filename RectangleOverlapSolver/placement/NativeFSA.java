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

	public void place(ArrayList<Rectangle2D> rectangles) {
		int n=rectangles.size();
		double[] x=new double[n];
		double[] y=new double[n];
		double[] w=new double[n];
		double[] h=new double[n];
		for(int i=0;i<n;i++) {
			Rectangle2D r=rectangles.get(i);
			x[i]=r.getMinX();
			y[i]=r.getMinY();
			w[i]=r.getWidth()+xgap;
			h[i]=r.getHeight()+ygap;
		}
		place(x,y,w,h);
		for(int i=0;i<n;i++) {
			Rectangle2D r=rectangles.get(i);
			r.setRect(x[i],y[i],r.getWidth(),r.getHeight());
		}
	}
}
