#ifndef BIN_HEAP
#define BIN_HEAP

class BinHeap
{
    int *data;
    int current_size, max_size;

    // TODO: remove this dirty hack
    int *test_matrix;

    void bubble_down(int index)
    {
        int left_child_index = 2 * index + 1;
        int right_child_index = 2 * index + 2;

        if (left_child_index > this->current_size) return;

        int min_index = index;

        if (this->test_matrix[this->data[index]] > this->test_matrix[this->data[left_child_index]]) min_index = left_child_index;
        // if (this->data[index] > this->data[left_child_index]) min_index = left_child_index;
        if ((right_child_index < this->current_size) && (this->test_matrix[this->data[min_index]] > this->test_matrix[this->data[right_child_index]])) min_index = right_child_index;
        // if ((right_child_index < this->current_size) && (this->data[min_index] > this->data[right_child_index])) min_index = right_child_index;

        if (min_index != index)
        {
            int temp = this->data[index];
            this->data[index] = this->data[min_index];
            this->data[min_index] = temp;
            bubble_down(min_index);
        }
    }

    void bubble_up(int index)
    {
        if (!index) return;

        int parent_index = (index - 1) / 2;

        // if (this->data[parent_index] > this->data[index])
        if (this->test_matrix[this->data[parent_index]] > this->test_matrix[this->data[index]])
        {
            int temp = this->data[parent_index];
            this->data[parent_index] = this->data[index];
            this->data[index] = temp;
            bubble_up(parent_index);
        }
    }

public:
    BinHeap(int max_size, int *matrix) // TODO: remove test matrix
    {
        this->test_matrix = matrix;
        this->max_size = max_size;
        this->data = (int *) malloc(this->max_size * sizeof(int));
        this->current_size = 0;
    }

    ~BinHeap()
    {
        free(this->data);
    }

    void insert(int value)
    {
        this->data[this->current_size] = value;
        this->current_size++;

        bubble_up(this->current_size - 1);
    }

    int get_next()
    {
        if (!this->current_size) return -1;

        return this->data[0];
    }
    
    void remove_next()
    {
        if (!this->current_size) return;

        this->data[0] = this->data[this->current_size - 1];
        this->current_size--;

        bubble_down(0);
    }

    int pop_next()
    {
        int next = this->get_next();

        this->remove_next();

        return next;
    }
};

#endif
