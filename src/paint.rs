use std::str::FromStr;

#[derive(Debug, Clone)]
pub struct ParsePaintModeError {
}

#[derive(Debug, Clone)]
pub enum PaintMode {
    Brush,
    Text,
    Rectangle,
    Ellipse,
    Arrow,
    Blur,
}

impl FromStr for PaintMode {
    type Err = ();

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s.to_lowercase().as_str() {
            "brush" => Ok(PaintMode::Brush),
            "text" => Ok(PaintMode::Text),
            "rectangle" => Ok(PaintMode::Rectangle),
            "ellipse" => Ok(PaintMode::Ellipse),
            "arrow" => Ok(PaintMode::Arrow),
            "blur" => Ok(PaintMode::Blur),
            _ => Err(()),
        }
    }
}
