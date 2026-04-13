import io
import torch
import torch.nn as nn
from torch.utils.data import Dataset
from torchvision import models, transforms
from PIL import Image

# Config
IMG_SIZE = 224
HAZARD_THRESHOLD = 0.5  # <= safe, > hazardous

# Dataset from dataframe with image bytes
class RoadConditionDataset(Dataset):
    def __init__(self, df, mode="train"):
        """
        mode: 'train' | 'val' | 'test'
        Uses TrainingID: 0=train, 1=val, 2=test
        """
        if mode == "train":
            self.df = df[df["TrainingID"] == 0].reset_index(drop=True)
            augment = True
        elif mode == "val":
            self.df = df[df["TrainingID"] == 1].reset_index(drop=True)
            augment = False
        else:
            self.df = df[df["TrainingID"] == 2].reset_index(drop=True)
            augment = False

        # Transforms: resize, normalize
        base_transforms = [
            transforms.Resize((IMG_SIZE, IMG_SIZE)),
            transforms.ToTensor(),
            transforms.Normalize(
                mean=[0.485, 0.456, 0.406],
                std=[0.229, 0.224, 0.225],
            ),
        ]
        if augment:
            self.transform = transforms.Compose([
                transforms.Resize((IMG_SIZE, IMG_SIZE)),
                transforms.RandomHorizontalFlip(),
                transforms.RandomRotation(15),
                transforms.ColorJitter(
                    brightness=0.2,
                    contrast=0.2,
                    saturation=0.2,
                    hue=0.05,
                ),
                transforms.ToTensor(),
                transforms.Normalize(
                    mean=[0.485, 0.456, 0.406],
                    std=[0.229, 0.224, 0.225],
                ),
            ])
        else:
            self.transform = transforms.Compose(base_transforms)

    def __len__(self):
        return len(self.df)

    def __getitem__(self, idx):
        row = self.df.iloc[idx]

        # Decode image bytes
        img_bytes = row["ImageBytes"] if "ImageBytes" in self.df.columns else row[1]
        if isinstance(img_bytes, str):
                        img_bytes = eval(img_bytes)
        img = Image.open(io.BytesIO(img_bytes)).convert("RGB")

        img = self.transform(img)

        hazard_level = float(row["HazardLevel"])

        # Binary label: 0 = safe, 1 = hazardous
        binary_label = 1 if hazard_level > HAZARD_THRESHOLD else 0

        # Outputs:
        #   binary_label: for safe vs hazardous
        #   hazard_level: for regression / multi-class
        return img, torch.tensor(binary_label, dtype=torch.long), torch.tensor(hazard_level, dtype=torch.float32)


# Model definition
class RoadConditionModel(nn.Module):
    def __init__(self, backbone_name="resnet18", pretrained=True):
        super().__init__()
        # Use ResNet18 as a simple CNN backbone
        backbone = models.resnet18(weights=models.ResNet18_Weights.DEFAULT if pretrained else None)

        num_feats = backbone.fc.in_features
        backbone.fc = nn.Identity()  # remove original classification head
        self.backbone = backbone

        # Head 1: binary classification (safe vs hazardous)
        self.fc_binary = nn.Linear(num_feats, 2)

        # Head 2: hazard regression (0–1)
        self.fc_hazard = nn.Linear(num_feats, 1)

    def forward(self, x):
        feats = self.backbone(x)
        logits_binary = self.fc_binary(feats)
        hazard_score = torch.sigmoid(self.fc_hazard(feats)).squeeze(1)  # 0–1
        return logits_binary, hazard_score
