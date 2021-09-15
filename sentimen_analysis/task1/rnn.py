import torch
from torchtext.legacy import data
from torchtext.legacy import datasets
import random
import torch.nn as nn
import torch.optim as optim
import time

#设置随机数种子
seed = 1234

#设置种子,神经网络中参数默认是进行随机初始化的
torch.manual_seed(seed)
#将这个 flag 置为True的话，每次返回的卷积算法将是确定的，即默认算法
#讲torch的随机数种子设置为固定值的话，可以保证每次运行网络的时候相同输入的输出是固定的
torch.backends.cudnn.deterministic = True

#读取数据和标签
text = data.Field(tokenize = "spacy",tokenizer_language="en_core_web_sm")
label = data.LabelField(dtype = torch.float)

#下载IMDb数据集并且将其拆分为规范的训练集和测试集，作为torchtext.datasets对象
train_data,test_data = datasets.IMDB.splits(text,label)

#查看训练集和测试集的数量
# print(f"Number of training examples: {len(train_data)}")
# print(f"Number of testing examples: {len(test_data)}")

#查看示例数据
# print(vars(train_data.examples[1]))

#将训练集随机划分测试集和验证集，split_ratio为0.8表示80%的示例构成训练集，20%构成验证集
#将我们之前设置的seed随机种子传递给random_state参数，确保我们每次都获得相同的训练集和验证集
train_data,valid_data = train_data.split(split_ratio=0.8,random_state = random.seed(seed))

#分别查看训练集，验证集，测试集分别有多少条数据
# print(f"Number of training examples: {len(train_data)}")
# print(f"Number of validation examples: {len(valid_data)}")
# print(f"Number of testing examples: {len(test_data)}")

#构建词汇表
max_vocab_size =25000

#给每个词标一个index
text.build_vocab(train_data,max_size = max_vocab_size)
label.build_vocab(train_data)

#设置验证集，测试集，训练集的迭代器，每一次迭代都会返回一个batch的数据
#可以将迭代器返回的张量放在GPU上
batch_size = 64
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
print(device)
train_iterator,valid_iterator,test_iterator = data.BucketIterator.splits((train_data,valid_data,test_data),batch_size = batch_size,device = device)



class RNN(nn.Module):
    def __init__(self,input_dim,embedding_dim,hidden_dim,output_dim):
        super().__init__()
        self.embedding = nn.Embedding(input_dim,embedding_dim)
        self.rnn = nn.RNN(embedding_dim,hidden_dim)
        #全连接层
        self.fc = nn.Linear(hidden_dim,output_dim)

    def forward(self,text):
        # text = [sent len,batch size],text是每一个句子对应的one-hot向量转换得到的
        embedded = self.embedding(text)
        # embedded = [sent len,batch size,emb dim]
        # output = [sent len, batch size, hid dim]
        # hidden = [1, batch size, hid dim]
        output,hidden = self.rnn(embedded)

        #判断RNN最后一个隐藏层的output和hidden是否相等
        assert torch.equal(output[-1,:,:],hidden.squeeze(0))

        return self.fc(hidden.squeeze(0))

input_dim = len(text.vocab)
embedding_dim = 100
hidden_dim = 256
output_dim = 1

model = RNN(input_dim, embedding_dim, hidden_dim, output_dim)

#训练模型,SGD优化器，随机梯度下降，model.parameters()表示需要更新的参数，lr为学习率
optimizer = optim.SGD(model.parameters(),lr=1e-3)
#定义损失函数,BCEWithLogitsLoss一般用来做二分类
criterion = nn.BCEWithLogitsLoss()
#将张量放在GPU上计算
model = model.to(device)
criterion = criterion.to(device)

#二分类的准确率
#torch.round将输入的张量每个元素舍入到最近的整数
#sigmoid函数输出为概率，如果不是这个标签的话，输出的概率就会很小，这样经过舍入之后，得到的值等于另一个标签
def binary_accuracy(preds,y):
    rounded_preds = torch.round(torch.sigmoid(preds))
    correct = (rounded_preds == y).float()
    acc = correct.sum() / len(correct)
    return acc

#训练函数,squeeze(0)代表若第一维度值为1则去除第一维度，squeeze(1)代表若第二维度值为1则去除第二维度
#
def train(model,iterator,optimizer,criterion):
     epoch_loss = 0
     epoch_acc = 0

     model.train()

     for batch in iterator:
         optimizer.zero_grad()
         predictions = model(batch.text).squeeze(1)
         loss = criterion(predictions,batch.label)
         acc = binary_accuracy(predictions,batch.label)
         #后向传播计算梯度
         loss.backward()
         #使用SGD来更新参数
         optimizer.step()

         epoch_loss += loss.item()
         epoch_acc += acc.item()

     return epoch_loss / len(iterator),epoch_acc / len(iterator)

def evaluate(model, iterator, criterion):
    
    epoch_loss = 0
    epoch_acc = 0
    
    model.eval()
    
    with torch.no_grad():
    
        for batch in iterator:

            predictions = model(batch.text).squeeze(1)
            
            loss = criterion(predictions, batch.label)
            
            acc = binary_accuracy(predictions, batch.label)

            epoch_loss += loss.item()
            epoch_acc += acc.item()
        
    return epoch_loss / len(iterator), epoch_acc / len(iterator)

def epoch_time(start_time,end_time):
    elapsed_time = end_time - start_time
    elapsed_mins = int(elapsed_time / 60)
    elapsed_secs = int(elapsed_time - (elapsed_mins * 60))
    return elapsed_mins, elapsed_secs

#训练过程
# n_epoch = 5
# best_valid_loss = float("inf")
# for epoch in range(n_epoch):
#     start_time = time.time()
#     train_loss,train_acc = train(model,train_iterator,optimizer,criterion)
#     valid_loss,valid_acc = evaluate(model,valid_iterator,criterion)

#     end_time = time.time()

#     epoch_mins,epoch_secs = epoch_time(start_time,end_time)

#     if valid_loss < best_valid_loss:
#         best_valid_loss = valid_loss
#         torch.save(model.state_dict(),"checkpoint/tut1-model.pt")

#     print(f"epoch:{epoch+1:02} | epoch time:{epoch_mins}m {epoch_secs}s")
#     print(f"\ttrain loss:{train_loss:.3f}| train acc:{train_acc * 100:.2f}%")
#     print(f"\tvalid loss:{valid_loss:.3f}| valid acc:{valid_acc * 100:.2f}%")

#测试过程

model.load_state_dict(torch.load("nlp/task1/checkpoint"))
test_loss,test_acc = evaluate(model,test_iterator,criterion)
print(f"test loss:{test_loss:.3f} | test acc:{test_acc*100:.2f}%")







    








