package placement;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.geom.Rectangle2D;
import java.awt.print.PageFormat;
import java.awt.print.PrinterJob;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectOutput;
import java.io.ObjectOutputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Hashtable;

import javax.swing.Box;
import javax.swing.ButtonGroup;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JRadioButton;
import javax.swing.JTextField;
import javax.swing.JToggleButton;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import placement.QPRectanglePlacement.Algorithm;
import placement.RectangleDrawerPanel.InteractionMode;

public class RectangleDrawerFrame extends JFrame {

	enum Algorithm {
		ACTIVESET, NATIVE_ACTIVESET, MOSEK, FSA
	}

	public RectangleDrawerFrame(String title) {
		super(title);
	}

	Algorithm algorithm = Algorithm.ACTIVESET;

	boolean completeConstraints = false;

	boolean orthogonalOrderingConstraints = false;

	boolean animate = false;

	boolean split = false;

	public static int DefaultXGap = 5;

	public static int DefaultYGap = 5;

	public RectangleDrawerPanel d;

	public static void main(String args[]) {
		final RectangleDrawerFrame f = new RectangleDrawerFrame(
				"Rectangle Drawer");
		f.init();
		if (args.length > 0) {
			System.out.println("Arg " + args[0]);
			f.d.load(new File(args[0]));
			//f.cleanup(DefaultXGap,DefaultYGap);
		}
	}

	public void init() {
		Box hBox1 = Box.createHorizontalBox();
		Box hBox2 = Box.createHorizontalBox();
		Box vBox = Box.createVerticalBox();
		d = new RectangleDrawerPanel();
		d.setSize(new Dimension(800, 600));
		d.setBackground(Color.WHITE);
		JButton cleanupButton = new JButton("Remove Overlaps");
		JButton clearButton = new JButton("Clear");
		JButton undoButton = new JButton("Undo");
		JButton loadButton = new JButton("Load");
		JButton saveButton = new JButton("Save");
		JButton randomButton = new JButton("Random");
		JButton printButton = new JButton("Print");
		JButton dumpButton = new JButton("screenDump");
		final JToggleButton moveButton = new JToggleButton("Move Rectangle");
		final JTextField xGapField = new JTextField("" + DefaultXGap);
		xGapField.setMaximumSize(new Dimension(100, 30));
		final JTextField yGapField = new JTextField("" + DefaultYGap);
		yGapField.setMaximumSize(new Dimension(100, 30));
		JCheckBox completeConstraintsCB = new JCheckBox("n^2");
		final JCheckBox orthogonalConstraintsCB = new JCheckBox("OO");
		final JCheckBox animateCB = new JCheckBox("Animate");
		final JCheckBox splitCB = new JCheckBox("Split");
		final JRadioButton activeSetRB = new JRadioButton("AS");
		activeSetRB.addActionListener(new ActionListener() {

			public void actionPerformed(ActionEvent arg0) {
				algorithm = Algorithm.ACTIVESET;
				splitCB.setEnabled(true);
				animateCB.setEnabled(true);
				orthogonalConstraintsCB.setEnabled(true);
			}

		});

		final JRadioButton nativeActiveSetRB = new JRadioButton("Native AS");
		nativeActiveSetRB.addActionListener(new ActionListener() {

			public void actionPerformed(ActionEvent arg0) {
				algorithm = Algorithm.NATIVE_ACTIVESET;
				splitCB.setEnabled(true);
				animateCB.setEnabled(false);
				orthogonalConstraintsCB.setEnabled(true);
			}

		});

		final JRadioButton mosekRB = new JRadioButton("QP");
		mosekRB.addActionListener(new ActionListener() {

			public void actionPerformed(ActionEvent arg0) {
				algorithm = Algorithm.MOSEK;
				splitCB.setEnabled(false);
				animateCB.setEnabled(false);
				orthogonalConstraintsCB.setEnabled(true);
			}

		});
		// mosekRB.setEnabled(false);
		final JRadioButton fsaRB = new JRadioButton("FSA");
		fsaRB.addActionListener(new ActionListener() {

			public void actionPerformed(ActionEvent arg0) {
				algorithm = Algorithm.FSA;
				splitCB.setEnabled(false);
				animateCB.setEnabled(false);
				orthogonalConstraintsCB.setEnabled(false);
			}

		});
		ButtonGroup bg = new ButtonGroup();
		bg.add(activeSetRB);
		bg.add(nativeActiveSetRB);
		bg.add(mosekRB);
		bg.add(fsaRB);
		switch (algorithm) {
		case ACTIVESET:
			activeSetRB.setSelected(true);
			break;
		case NATIVE_ACTIVESET:
			nativeActiveSetRB.setSelected(true);
			break;
		case MOSEK:
			mosekRB.setSelected(true);
			break;
		case FSA:
			fsaRB.setSelected(true);
			break;
		}
		completeConstraintsCB.setSelected(completeConstraints);
		orthogonalConstraintsCB.setSelected(orthogonalOrderingConstraints);
		animateCB.setSelected(animate);
		splitCB.setSelected(split);
		hBox2.add(cleanupButton);
		hBox2.add(xGapField);
		hBox2.add(yGapField);
		hBox1.add(clearButton);
		hBox1.add(randomButton);
		hBox1.add(undoButton);
		hBox1.add(loadButton);
		hBox1.add(saveButton);
		hBox1.add(printButton);
		hBox1.add(dumpButton);
		hBox1.add(moveButton);
		// hBox.add(completeConstraintsCB);
		hBox2.add(orthogonalConstraintsCB);
		hBox2.add(animateCB);
		hBox2.add(splitCB);
		hBox2.add(activeSetRB);
		hBox2.add(nativeActiveSetRB);
		hBox2.add(mosekRB);
		hBox2.add(fsaRB);
		vBox.add(d);
		vBox.add(hBox1);
		vBox.add(hBox2);
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		add(vBox);
		setSize(700, 700);
		setVisible(true);
		cleanupButton.addActionListener(new ActionListener() {

			public void actionPerformed(ActionEvent arg0) {
				cleanup(Integer.parseInt(xGapField.getText()),Integer.parseInt(yGapField.getText()));
			}

		});
		clearButton.addActionListener(new ActionListener() {

			public void actionPerformed(ActionEvent arg0) {
				d.backup();
				d.clear();
			}

		});
		undoButton.addActionListener(new ActionListener() {

			public void actionPerformed(ActionEvent arg0) {
				d.undo();
				d.repaint();
			}

		});
		final RectangleDrawerFrame f = this;
		loadButton.addActionListener(new ActionListener() {

			public void actionPerformed(ActionEvent arg0) {
				BlocksFileFilter ff = d.getFileFilter();
				JFileChooser chooser = new JFileChooser(".");
				chooser.addChoosableFileFilter(ff);
				chooser.setFileFilter(ff);
				if (ff.lastSelectedFile != null) {
					chooser.setSelectedFile(ff.lastSelectedFile);
				}
				int returnVal = chooser.showOpenDialog(f);
				if (returnVal == JFileChooser.APPROVE_OPTION) {
					File f = chooser.getSelectedFile();
					d.load(f);
				}
			}
		});
		randomButton.addActionListener(new ActionListener() {

			public void actionPerformed(ActionEvent arg0) {
				d.generateRandom(5);
			}
		});
		printButton.addActionListener(new ActionListener() {

			public void actionPerformed(ActionEvent arg0) {
				PrinterJob printJob = PrinterJob.getPrinterJob();
				printJob.setPrintable(d);
				PageFormat pf = printJob.pageDialog(printJob.defaultPage());
				if (printJob.printDialog()) {
					try {
						printJob.print();
					} catch (Exception ex) {
					}
				}
			}
		});
		dumpButton.addActionListener(new ActionListener() {

			public void actionPerformed(ActionEvent arg0) {
				d.saveAsJPEG("C:/Temp/Test.jpg");
			}
		});
		moveButton.addChangeListener(new ChangeListener() {
			public void stateChanged(ChangeEvent ce) {
				if(moveButton.isSelected()) {
					d.setInteractionMode(RectangleDrawerPanel.InteractionMode.Select);
				} else {
					d.setInteractionMode(RectangleDrawerPanel.InteractionMode.Create);
				}
			}
		});
		saveButton.addActionListener(new ActionListener() {

			public void actionPerformed(ActionEvent arg0) {
				BlocksFileFilter ff = d.getFileFilter();
				JFileChooser chooser = new JFileChooser(".");
				chooser.setFileFilter(ff);
				if (ff.lastSelectedFile != null) {
					chooser.setSelectedFile(ff.lastSelectedFile);
				}
				int returnVal = chooser.showSaveDialog(f);
				if (returnVal == JFileChooser.APPROVE_OPTION) {
					ff.lastSelectedFile = d.save(chooser.getSelectedFile().getPath());
				}
			}
		});
		completeConstraintsCB.addChangeListener(new ChangeListener() {

			public void stateChanged(ChangeEvent arg0) {
				completeConstraints = completeConstraints ? false : true;
			}

		});
		orthogonalConstraintsCB.addChangeListener(new ChangeListener() {

			public void stateChanged(ChangeEvent arg0) {
				orthogonalOrderingConstraints = orthogonalOrderingConstraints ? false
						: true;
			}

		});
		animateCB.addChangeListener(new ChangeListener() {

			public void stateChanged(ChangeEvent arg0) {
				animate = animate ? false : true;
			}

		});
		splitCB.addChangeListener(new ChangeListener() {

			public void stateChanged(ChangeEvent arg0) {
				split = split ? false : true;
			}

		});

		d.updateUI();
	}

	public void cleanup(int xGap, int yGap) {
		d.backup();
		if (algorithm == Algorithm.FSA) {
			FSA r = new FSA(xGap, yGap);
			r.place(d.rectangles);
		} else {
			QPRectanglePlacement.Algorithm qalg = null;
			switch (algorithm) {
			case NATIVE_ACTIVESET:
				qalg = QPRectanglePlacement.Algorithm.CACTIVESET;
				break;
			case MOSEK:
				qalg = QPRectanglePlacement.Algorithm.MOSEK;
				break;
			default:
				qalg = QPRectanglePlacement.Algorithm.ACTIVESET;
			}
			QPRectanglePlacement r = new QPRectanglePlacement(split,
					completeConstraints, orthogonalOrderingConstraints, qalg,
					xGap, yGap, animate);
			if (animate) {
				r.addObserver(d);
			}
			r.place(d.rectangles);
		}
		d.fitToScreen();
		d.repaint();
	}
}
