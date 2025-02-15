use std::collections::HashMap;
use std::env;
use std::fs::File;
use std::io::{Read, Write};
use plotters::prelude::*;
use std::hash::{Hash, Hasher};

type Point = (f64, f64);

#[derive(Debug, Clone, Copy, PartialEq, PartialOrd)]
struct FloatKey(f64);

impl Eq for FloatKey {}

impl Hash for FloatKey {
    fn hash<H: Hasher>(&self, state: &mut H) {
        // Convert `f64` to bits and hash them (ignores `NaN` edge cases)
        self.0.to_bits().hash(state);
    }
}

/// Reads a file that may be UTF-16 (LE or BE) or UTF-8.
/// It checks for a BOM and decodes accordingly, then extracts lines of two f64 values.
fn read_data(file_path: &str) -> Vec<Point> {
    let path = std::path::Path::new(file_path);
    if !path.exists() {
        eprintln!("Error: File does not exist at path: {}", file_path);
        std::process::exit(1);
    }
    let mut file = File::open(path).expect("Failed to open file");
    let mut buffer = Vec::new();
    file.read_to_end(&mut buffer)
        .expect("Failed to read file");

    let content = if buffer.starts_with(&[0xFF, 0xFE]) {
        // UTF-16 LE: skip BOM and decode
        let mut u16_buffer = Vec::with_capacity(buffer.len() / 2);
        for chunk in buffer[2..].chunks_exact(2) {
            let val = u16::from_le_bytes([chunk[0], chunk[1]]);
            u16_buffer.push(val);
        }
        String::from_utf16(&u16_buffer).expect("Invalid UTF-16 LE data")
    } else if buffer.starts_with(&[0xFE, 0xFF]) {
        // UTF-16 BE: skip BOM and decode
        let mut u16_buffer = Vec::with_capacity(buffer.len() / 2);
        for chunk in buffer[2..].chunks_exact(2) {
            let val = u16::from_be_bytes([chunk[0], chunk[1]]);
            u16_buffer.push(val);
        }
        String::from_utf16(&u16_buffer).expect("Invalid UTF-16 BE data")
    } else {
        // Assume UTF-8 if no BOM is found.
        String::from_utf8(buffer).expect("Invalid UTF-8 data")
    };

    let mut points = Vec::new();
    for line in content.lines() {
        let nums: Vec<f64> = line
            .split_whitespace()
            .filter_map(|s| s.parse::<f64>().ok())
            .collect();
        if nums.len() == 2 {
            points.push((nums[0], nums[1]));
        }
    }
    points
}

fn calculate_regression(points: &Vec<Point>) -> (f64, f64, f64) {
    let n = points.len() as f64;
    let (mut sx, mut sy, mut sxy, mut sxx) = (0.0, 0.0, 0.0, 0.0);

    for &(x, y) in points {
        sx += x;
        sy += y;
        sxx += x * x;
        sxy += x * y;
    }

    let m = (n * sxy - sx * sy) / (n * sxx - sx * sx);
    let b = (sy - m * sx) / n;
    let r = sxy / (sx * sy);

    (m, b, r)
}

fn calculate_residual(points: &Vec<Point>, m: f64, b: f64) -> (Vec<f64>, Vec<f64>) {
    let mut x_set: HashMap<FloatKey, Vec<f64>> = HashMap::new();
    let mut residual_x = Vec::new();
    let mut residual_y = Vec::new();

    for &(x, y) in points {
        x_set.entry(FloatKey(x)).or_insert_with(Vec::new).push(y);
    }

    for (&FloatKey(x), y_values) in &x_set {
        let y_hat = y_values.iter().sum::<f64>() / y_values.len() as f64;
        residual_x.push(x);
        residual_y.push(y_hat - (m * x + b));
    }

    (residual_x, residual_y)
}

fn plot_data(points: &Vec<Point>, m: f64, b: f64) -> Result<(), Box<dyn std::error::Error>> {
    // Set up the drawing area.
    let root = BitMapBackend::new("out/plot.png", (800, 600)).into_drawing_area();
    root.fill(&WHITE)?;

    // Automatically determine bounds.
    let x_min = points.iter().map(|p| p.0).fold(f64::INFINITY, f64::min);
    let x_max = points.iter().map(|p| p.0).fold(f64::NEG_INFINITY, f64::max);
    let y_min = points.iter().map(|p| p.1).fold(f64::INFINITY, f64::min);
    let y_max = points.iter().map(|p| p.1).fold(f64::NEG_INFINITY, f64::max);

    let x_margin = (x_max - x_min) * 0.1;
    let y_margin = (y_max - y_min) * 0.1;

    let mut chart = ChartBuilder::on(&root)
        .caption("Linear Regression", ("sans-serif", 50))
        .margin(10)
        .x_label_area_size(30)
        .y_label_area_size(30)
        .build_cartesian_2d(
            (x_min - x_margin)..(x_max + x_margin),
            (y_min - y_margin)..(y_max + y_margin),
        )?;

    chart.configure_mesh().draw()?;

    let x_data: Vec<f64> = points.iter().map(|p| p.0).collect();
    let y_data: Vec<f64> = points.iter().map(|p| p.1).collect();

    // Plot the scatter points.
    chart.draw_series(PointSeries::of_element(
        x_data.iter().copied().zip(y_data.iter().copied()),
        5,         // The point size (u32)
        &RED,      // The base style (converted into a ShapeStyle)
        &|(x, y): (f64, f64), size: u32, style: ShapeStyle| {
            EmptyElement::at((x, y))
                + Circle::new((0, 0), size, style.filled())
        },
    ))?;


    // Generate the regression line.
    let line_x: Vec<f64> = (0..=1000)
        .map(|i| x_min + i as f64 * (x_max - x_min) / 1000.0)
        .collect();
    let line_y: Vec<f64> = line_x.iter().map(|&x| m * x + b).collect();

    chart
        .draw_series(LineSeries::new(
            line_x.iter().zip(line_y.iter()).map(|(&x, &y)| (x, y)),
            &BLUE,
        ))?
        .label(format!("y = {:.2}x + {:.2}", m, b))
        .legend(|(x, y)| {
            PathElement::new(vec![(x, y), (x + 10, y)], &BLUE)
        });

    chart.configure_series_labels().draw()?;

    Ok(())
}

fn plot_residuals(
    residual_x: &Vec<f64>,
    residual_y: &Vec<f64>,
) -> Result<(), Box<dyn std::error::Error>> {
    let root = BitMapBackend::new("out/residuals.png", (800, 600)).into_drawing_area();
    root.fill(&WHITE)?;

    let x_min = residual_x.iter().fold(f64::INFINITY, |a, &b| f64::min(a, b));
    let x_max = residual_x.iter().fold(f64::NEG_INFINITY, |a, &b| f64::max(a, b));
    let y_min = residual_y.iter().fold(f64::INFINITY, |a, &b| f64::min(a, b));
    let y_max = residual_y.iter().fold(f64::NEG_INFINITY, |a, &b| f64::max(a, b));

    let x_margin = (x_max - x_min) * 0.1;
    let y_margin = (y_max - y_min) * 0.1;

    let mut chart = ChartBuilder::on(&root)
        .caption("Residuals", ("sans-serif", 50))
        .margin(10)
        .x_label_area_size(30)
        .y_label_area_size(30)
        .build_cartesian_2d(
            (x_min - x_margin)..(x_max + x_margin),
            (y_min - y_margin)..(y_max + y_margin),
        )?;

    chart.configure_mesh().draw()?;

   chart.draw_series(PointSeries::of_element(
        residual_x.iter().copied().zip(residual_y.iter().copied()),
        5,         // The point size
        &GREEN,    // The base style
        &|(x, y): (f64, f64), size: u32, style: ShapeStyle| {
            EmptyElement::at((x, y))
                + Circle::new((0, 0), size, style.filled())
        },
    ))?;


    Ok(())
}

fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() < 3 {
        println!("Usage: {} [testdata_file] [output_file]", args[0]);
        return;
    }

    let testdata_file = &args[1];
    let output_file = &args[2];

    println!("Reading data from: {}", testdata_file);

    let points = read_data(testdata_file);
    if points.is_empty() {
        eprintln!("No valid points found in the file.");
        return;
    }

    let (m, b, r) = calculate_regression(&points);
    let out_filename = format!("out/{}", output_file);
    let mut file = File::create(out_filename).expect("Unable to create output file");
    writeln!(
        file,
        "y = {:.2}x + {:.2}\nStrength (r): {:.2}",
        m, b, r
    )
    .unwrap();

    println!("Regression line: y = {:.2}x + {:.2}", m, b);
    println!("Accuracy: {:.2}", r);

    plot_data(&points, m, b).expect("Failed to plot data");

    let (residual_x, residual_y) = calculate_residual(&points, m, b);
    plot_residuals(&residual_x, &residual_y).expect("Failed to plot residuals");

    println!("Plots saved as plot.png and residuals.png");
}

